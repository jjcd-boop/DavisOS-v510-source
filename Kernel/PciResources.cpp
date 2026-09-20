#include "PciResources.hpp"
#include "Pci.hpp"
#include "DeviceResourceBroker.hpp"
namespace Davis::PciResources {
static u32 count=0;
static u64 devId(const Pci::Device&d){return 0xD000000000000000ull|((u64)d.vendor<<32)|((u64)d.device<<16)|((u64)d.bus<<8)|((u64)d.slot<<3)|d.function;}
static bool supported(const Pci::Device&d){return d.classCode==0x01||d.classCode==0x02||d.classCode==0x03||(d.classCode==0x04&&d.subclass==0x03)||(d.classCode==0x0c&&d.subclass==0x03);}
static void publishInterruptCaps(const Pci::Device&d){ u32 st=Pci::ReadConfig(d.bus,d.slot,d.function,0x04); if(!(st&(1u<<20)))return; u8 cap=(u8)(Pci::ReadConfig(d.bus,d.slot,d.function,0x34)&0xfc); for(u32 guard=0;cap>=0x40&&guard++<48;){u32 h=Pci::ReadConfig(d.bus,d.slot,d.function,cap);u8 id=(u8)h,next=(u8)((h>>8)&0xfc);if(id==0x05){if(DeviceResourceBroker::Register(DeviceResourceBroker::Type::Msi,devId(d),cap,1,DeviceResourceBroker::RightInspect|DeviceResourceBroker::RightSignal))count++;break;}if(!next||next==cap)break;cap=next;} }
static void publishBars(const Pci::Device&d){
 u32 cmd=Pci::ReadConfig(d.bus,d.slot,d.function,0x04);Pci::WriteConfig(d.bus,d.slot,d.function,0x04,cmd&~3u);
 for(u8 off=0x10;off<=0x24;off+=4){u32 orig=Pci::ReadConfig(d.bus,d.slot,d.function,off);if(!orig||orig==0xffffffffu)continue;
  if(orig&1u){Pci::WriteConfig(d.bus,d.slot,d.function,off,0xffffffffu);u32 mask=Pci::ReadConfig(d.bus,d.slot,d.function,off);Pci::WriteConfig(d.bus,d.slot,d.function,off,orig);u32 m=mask&~3u;if(!m)continue;u64 len=(u64)(~m)+1ull;u64 base=orig&~3u;if(base&&len&&DeviceResourceBroker::Register(DeviceResourceBroker::Type::IoPort,devId(d),base,len,DeviceResourceBroker::RightInspect|DeviceResourceBroker::RightMap))count++;continue;}
  u32 type=(orig>>1)&3u;if(type==2&&off<=0x20){u32 hi=Pci::ReadConfig(d.bus,d.slot,d.function,off+4);Pci::WriteConfig(d.bus,d.slot,d.function,off,0xffffffffu);Pci::WriteConfig(d.bus,d.slot,d.function,off+4,0xffffffffu);u32 ml=Pci::ReadConfig(d.bus,d.slot,d.function,off);u32 mh=Pci::ReadConfig(d.bus,d.slot,d.function,off+4);Pci::WriteConfig(d.bus,d.slot,d.function,off,orig);Pci::WriteConfig(d.bus,d.slot,d.function,off+4,hi);u64 mask=((u64)mh<<32)|(ml&~0xfull);u64 base=((u64)hi<<32)|(orig&~0xfull);if(mask&&base){u64 len=(~mask)+1ull;if(len&&DeviceResourceBroker::Register(DeviceResourceBroker::Type::Mmio,devId(d),base,len,DeviceResourceBroker::RightInspect|DeviceResourceBroker::RightMap))count++;}off+=4;continue;}
  Pci::WriteConfig(d.bus,d.slot,d.function,off,0xffffffffu);u32 mask=Pci::ReadConfig(d.bus,d.slot,d.function,off);Pci::WriteConfig(d.bus,d.slot,d.function,off,orig);u32 m=mask&~0x0fu;if(!m)continue;u64 len=(u64)(~m)+1ull;u64 base=orig&~0x0fu;if(base&&len&&DeviceResourceBroker::Register(DeviceResourceBroker::Type::Mmio,devId(d),base,len,DeviceResourceBroker::RightInspect|DeviceResourceBroker::RightMap))count++;
 }
 Pci::WriteConfig(d.bus,d.slot,d.function,0x04,cmd);
 publishInterruptCaps(d);
 u32 il=Pci::ReadConfig(d.bus,d.slot,d.function,0x3c);u8 irq=(u8)il;if(irq!=0xff&&irq!=0&&DeviceResourceBroker::Register(DeviceResourceBroker::Type::Irq,devId(d),irq,1,DeviceResourceBroker::RightInspect|DeviceResourceBroker::RightSignal))count++;
}
void PublishAll(){count=0;Pci::State p{};Pci::Scan(p);for(u32 i=0;i<p.count;i++)if(supported(p.devices[i]))publishBars(p.devices[i]);}
u32 PublishedCount(){return count;}
}
