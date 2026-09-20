#include "Pci.hpp"
#include "../Arch/x86_64/Io.hpp"
namespace Davis::Pci {
u32 ReadConfig(u8 b,u8 s,u8 f,u8 off){u32 a=0x80000000u|((u32)b<<16)|((u32)s<<11)|((u32)f<<8)|(off&0xFC);Io::Out32(0xCF8,a);return Io::In32(0xCFC);}
void WriteConfig(u8 b,u8 s,u8 f,u8 off,u32 v){u32 a=0x80000000u|((u32)b<<16)|((u32)s<<11)|((u32)f<<8)|(off&0xFC);Io::Out32(0xCF8,a);Io::Out32(0xCFC,v);}
void EnableMemoryBusMaster(const Device&d){u32 v=ReadConfig(d.bus,d.slot,d.function,0x04);v|=(1u<<1)|(1u<<2);WriteConfig(d.bus,d.slot,d.function,0x04,v);}
u64 MemoryBarAddress(const Device&d,u8 index){if(index>=6)return 0;u8 off=(u8)(0x10+index*4);u32 lo=ReadConfig(d.bus,d.slot,d.function,off);if(!lo||lo==0xffffffffu||(lo&1u))return 0;u64 base=(u64)(lo&~0xfull);u32 type=(lo>>1)&3u;if(type==2u){if(index>=5)return 0;u32 hi=ReadConfig(d.bus,d.slot,d.function,(u8)(off+4));if(hi==0xffffffffu)return 0;base|=(u64)hi<<32;}return base;}
void Scan(State&st){st={};for(u32 b=0;b<256&&st.count<128;b++)for(u32 s=0;s<32&&st.count<128;s++)for(u32 f=0;f<8&&st.count<128;f++){u32 id=ReadConfig(b,s,f,0);if((id&0xFFFF)==0xFFFF){if(f==0)break;continue;}u32 c=ReadConfig(b,s,f,8);Device d{(u8)b,(u8)s,(u8)f,(u16)(id&0xFFFF),(u16)(id>>16),(u8)(c>>24),(u8)(c>>16),(u8)(c>>8),ReadConfig(b,s,f,0x10)};st.devices[st.count++]=d;if(d.classCode==0x0C&&d.subclass==0x03&&d.progIf==0x30&&!st.xhciFound){st.xhciFound=true;st.xhci=d;}
if(d.classCode==0x01){st.storageControllerCount++;}
if(d.classCode==0x01&&d.subclass==0x08&&d.progIf==0x02&&!st.nvmeFound){st.nvmeFound=true;st.nvme=d;}
if(d.classCode==0x01&&d.subclass==0x06&&d.progIf==0x01&&!st.ahciFound){st.ahciFound=true;st.ahci=d;}
if(d.classCode==0x01&&d.subclass==0x04&&!st.raidFound){st.raidFound=true;st.raid=d;}
// Intel VMD commonly presents the downstream NVMe path behind an Intel mass-storage/RAID function.
// This is classification only: never bind or write the controller solely from this heuristic.
if(d.vendor==0x8086&&d.classCode==0x01&&(d.subclass==0x04||d.subclass==0x08||d.subclass==0x80)&&!st.intelVmdCandidateFound){st.intelVmdCandidateFound=true;st.intelVmdCandidate=d;}
if(d.classCode==0x04&&d.subclass==0x03&&!st.hdaFound){st.hdaFound=true;st.hda=d;}
// PCI class 0x0C/0x80 is commonly used by Intel serial-I/O/I2C controllers.
// Keep this as discovery only: ACPI namespace parsing is still required before binding an I2C-HID device.
if(d.classCode==0x0C&&d.subclass==0x80){st.i2cCount++;if(!st.i2cFound){st.i2cFound=true;st.i2c=d;}}
if(d.classCode==0x02&&!st.networkFound){st.networkFound=true;st.network=d;}
if(d.classCode==0x02&&d.subclass==0x00&&!st.ethernetFound){st.ethernetFound=true;st.ethernet=d;}
if(d.classCode==0x03&&!st.displayFound){st.displayFound=true;st.display=d;}u32 h=ReadConfig(b,s,f,0x0C);if(f==0&&!(h&0x00800000))break;}}
}
