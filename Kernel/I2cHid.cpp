#include "I2cHid.hpp"
#include "AcpiTables.hpp"
namespace Davis::I2cHid {
State state{};
static volatile u32* R(uptr b,u32 o){return (volatile u32*)(b+o);} static u32 rd(uptr b,u32 o){return *R(b,o);} static void wr(uptr b,u32 o,u32 v){*R(b,o)=v;}
static uptr mmioBar(const Pci::Device&d,u8 index){u8 off=(u8)(0x10+index*4);u32 lo=Pci::ReadConfig(d.bus,d.slot,d.function,off);if(!lo||lo==0xffffffffu||(lo&1))return 0;u32 type=(lo>>1)&3u;if(type==1u||type==3u)return 0;u64 a=(u64)(lo&~0xFu);if(type==2u&&index<5){u32 hi=Pci::ReadConfig(d.bus,d.slot,d.function,(u8)(off+4));a|=((u64)hi<<32);}return (uptr)a;}
static bool knownIntelI2c(u16 id){return (id>=0xA368&&id<=0xA36B)||(id>=0x9DE8&&id<=0x9DEB)||(id>=0x34E8&&id<=0x34EB);}
static bool selectDw(const Pci::State&p,Pci::Device&out,uptr&base,u8&bar,u32&component){
 for(u32 i=0;i<p.count;i++){const auto&d=p.devices[i];bool serialClass=(d.classCode==0x0C&&d.subclass==0x80);bool intelKnown=(d.vendor==0x8086&&knownIntelI2c(d.device));if(!serialClass&&!intelKnown)continue;state.candidatesChecked++;Pci::EnableMemoryBusMaster(d);
  for(u8 bi=0;bi<2;bi++){uptr b=mmioBar(d,bi);if(!b)continue;u32 c=rd(b,0xfc);if(c==0x44570140u||c==0x44570110u||c==0x44570120u){out=d;base=b;bar=bi;component=c;return true;}}
 }return false;
}
// ACPI 6.x Large Resource item 0x8E = Serial Bus Connection descriptor.
// Common header: tag,len16,revision,resource-source-index,serial-bus-type,
// general-flags,type-specific-flags16,type-revision,type-data-length16.
// I2C type-specific data begins with ConnectionSpeed(u32), SlaveAddress(u16).
// v1.22 incorrectly treated bytes 11/12 as the slave address; those bytes are
// part of TypeDataLength/ConnectionSpeed. v1.23 decodes the defined layout and
// searches all checksum-valid DSDT/SSDT AML tables.
static bool scanI2cResourceTable(const u8*b,u32 len,u16&addr){
 if(!b||len<36||len>(4u<<20))return false;
 for(u32 i=36;i+18<=len;i++){
  if(b[i]!=0x8e)continue;
  u16 payload=(u16)b[i+1]|((u16)b[i+2]<<8);
  if(payload<15||i+3u+payload>len)continue;
  if(b[i+5]!=1)continue; // SerialBusType = I2C
  u16 typeLen=(u16)b[i+10]|((u16)b[i+11]<<8);
  if(typeLen<6)continue;
  u16 slave=(u16)b[i+16]|((u16)b[i+17]<<8);
  if(slave>=0x08&&slave<=0x77){addr=slave;return true;}
 }
 return false;
}
static bool findI2cResource(const Acpi::State&a,u16&addr){
 if(a.dsdtPresent&&a.dsdt){const u8*b=(const u8*)a.dsdt;u32 n=*(const u32*)(b+4);if(scanI2cResourceTable(b,n,addr))return true;}
 for(u32 i=0;i<a.count;i++){const auto&t=a.tables[i];if(!t.checksumOk||t.sig[0]!='S'||t.sig[1]!='S'||t.sig[2]!='D'||t.sig[3]!='T')continue;const u8*b=(const u8*)t.address;if(scanI2cResourceTable(b,t.length,addr))return true;}
 return false;
}
static bool waitClear(uptr b,u32 mask,u32 loops=200000){while(loops--){if(!(rd(b,0x70)&mask))return true;__asm__ __volatile__("pause");}return false;}
static bool waitSet(uptr b,u32 mask,u32 loops=200000){while(loops--){if(rd(b,0x70)&mask)return true;__asm__ __volatile__("pause");}return false;}
static bool disable(uptr b){wr(b,0x6c,0);for(u32 i=0;i<200000;i++){if(!(rd(b,0x9c)&1))return true;__asm__ __volatile__("pause");}return false;}
static bool enable(uptr b){wr(b,0x6c,1);for(u32 i=0;i<200000;i++){if(rd(b,0x9c)&1)return true;__asm__ __volatile__("pause");}return false;}
static bool readReg(uptr b,u16 dev,u16 reg,u8*out,u32 n){if(!n||n>64||!disable(b))return false;wr(b,0x04,dev);wr(b,0x00,0x65);wr(b,0x14,0x20);wr(b,0x18,0x20);wr(b,0x1c,0x28);wr(b,0x20,0x28);wr(b,0x24,0x28);wr(b,0x38,32);wr(b,0x3c,32);if(!enable(b))return false;wr(b,0x10,(u8)reg);wr(b,0x10,(u8)(reg>>8));for(u32 i=0;i<n;i++)wr(b,0x10,0x100|(i+1==n?0x200:0));u32 got=0,loops=400000;while(got<n&&loops--){u32 st=rd(b,0x70);if(st&0x40){state.errors++;disable(b);return false;}if(st&0x08)out[got++]=(u8)rd(b,0x10);__asm__ __volatile__("pause");}state.transactions++;return got==n;}
void Init(const BootInfo&bi){state={};Pci::State p{};Pci::Scan(p);if(!p.i2cFound){state.stage=Stage::NoController;return;}state.controllerFound=true;Acpi::State ac{};Acpi::Discover(bi,ac);AmlLite::State am{};AmlLite::Scan(ac,am);for(u32 i=0;i<am.hidCount;i++)if(am.devices[i].likelyTouchpad){state.acpiCandidate=true;for(u32 n=0;n<15;n++){state.hid[n]=am.devices[i].hid[n];if(!state.hid[n])break;}break;}if(!state.acpiCandidate){state.stage=Stage::NoAcpiDevice;return;}state.stage=Stage::CandidateFound;if(!findI2cResource(ac,state.targetAddress)){state.stage=Stage::Fault;return;}Pci::Device chosen{};if(!selectDw(p,chosen,state.mmio,state.barIndex,state.componentType)){state.stage=Stage::MmioInvalid;return;}state.pciVendor=chosen.vendor;state.pciDevice=chosen.device;state.pciBus=chosen.bus;state.pciSlot=chosen.slot;state.pciFunction=chosen.function;state.dwVerified=true;state.stage=Stage::ControllerReady;
 // Microsoft HID-over-I2C commonly obtains this register from _DSM. Until full AML _DSM evaluation lands,
 // probe only the specification-common register 0x0001 after both ACPI device and DW controller validation.
 state.hidDescReg=1;u8 h[30]{};if(!readReg(state.mmio,state.targetAddress,state.hidDescReg,h,sizeof(h))){state.stage=Stage::ControllerReady;return;}u16 l=(u16)h[0]|((u16)h[1]<<8),ver=(u16)h[2]|((u16)h[3]<<8);if(l<30||ver==0||ver>0x0200){state.stage=Stage::ControllerReady;return;}state.maxInput=(u16)h[10]|((u16)h[11]<<8);state.hidDescriptor=true;state.stage=Stage::HidDescriptorReady;}
bool Poll(i32&dx,i32&dy,bool&left){dx=dy=0;if(state.stage!=Stage::HidDescriptorReady&&state.stage!=Stage::Active)return false; // Descriptor/report parsing is intentionally bounded to mouse-compatible reports.
 // HID descriptor bytes must be re-read to obtain input register; avoid persistent hidden assumptions.
 u8 h[30]{};if(!readReg(state.mmio,state.targetAddress,state.hidDescReg,h,sizeof(h)))return false;u16 in=(u16)h[8]|((u16)h[9]<<8);u16 mx=(u16)h[10]|((u16)h[11]<<8);if(!in||mx<3||mx>64)return false;u8 r[64]{};u32 n=mx;if(!readReg(state.mmio,state.targetAddress,in,r,n))return false;u16 bytes=(u16)r[0]|((u16)r[1]<<8);if(bytes<5||bytes>n)return false; // common relative-pointer fallback; full report-descriptor parser follows.
 u32 o=2;if(bytes>=6&&r[o]<=16)o++;left=(r[o]&1)!=0;dx=(i8)r[o+1];dy=(i8)r[o+2];state.dx=dx;state.dy=dy;state.left=left;state.reports++;state.stage=Stage::Active;return true;}
const char* StageName(Stage s){switch(s){case Stage::NoController:return "NO I2C CONTROLLER";case Stage::NoAcpiDevice:return "NO ACPI HID DEVICE";case Stage::CandidateFound:return "I2C-HID CANDIDATE";case Stage::MmioInvalid:return "I2C MMIO UNSUPPORTED";case Stage::ControllerReady:return "I2C CONTROLLER READY";case Stage::HidDescriptorReady:return "HID DESCRIPTOR READY";case Stage::Active:return "I2C-HID ACTIVE";case Stage::Fault:return "I2C RESOURCE UNRESOLVED";default:return "DISABLED";}}
}
