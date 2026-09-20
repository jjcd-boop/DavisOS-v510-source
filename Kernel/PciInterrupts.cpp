#include "PciInterrupts.hpp"
#include "DeviceResourceBroker.hpp"
#include "DriverManager.hpp"
#include "DriverGateway.hpp"
#include "Pci.hpp"
#include "InterruptController.hpp"
#include "PlatformBootstrap.hpp"
namespace Davis::PciInterrupts {
extern "C" void DavisIrq50(); extern "C" void DavisIrq51(); extern "C" void DavisIrq52(); extern "C" void DavisIrq53();
extern "C" void DavisIrq54(); extern "C" void DavisIrq55(); extern "C" void DavisIrq56(); extern "C" void DavisIrq57();
extern "C" void DavisIrq58(); extern "C" void DavisIrq59(); extern "C" void DavisIrq5A(); extern "C" void DavisIrq5B();
extern "C" void DavisIrq5C(); extern "C" void DavisIrq5D(); extern "C" void DavisIrq5E(); extern "C" void DavisIrq5F();
struct Slot{u64 owner,resource;u8 vector,bus,slot,function,cap;bool active;}; static Slot slots[16]{};
static bool decode(u64 id,u8&b,u8&s,u8&f){if((id>>60)!=0xD)return false;b=(u8)(id>>8);s=(u8)((id>>3)&0x1f);f=(u8)(id&7);return true;}
void InstallGates(Idt::Table&t,u16 kc){void(*h[16])()={DavisIrq50,DavisIrq51,DavisIrq52,DavisIrq53,DavisIrq54,DavisIrq55,DavisIrq56,DavisIrq57,DavisIrq58,DavisIrq59,DavisIrq5A,DavisIrq5B,DavisIrq5C,DavisIrq5D,DavisIrq5E,DavisIrq5F};for(u32 i=0;i<16;i++)Idt::SetInterruptGate(t,(u8)(FirstVector+i),h[i],kc,0);}
bool ConfigureMsi(const Process::Image&p,DeviceResourceBroker::Resource&r,u8*out){if(r.type!=DeviceResourceBroker::Type::Msi||r.ownerProcessId!=p.id||!out)return false;u8 b,s,f;if(!decode(r.deviceId,b,s,f))return false;Slot*sl=nullptr;for(auto&x:slots)if(x.active&&x.owner==p.id&&x.resource==r.id){sl=&x;break;}if(!sl)for(auto&x:slots)if(!x.active){sl=&x;break;}if(!sl)return false;u8 cap=(u8)r.base;if(cap<0x40)return false;u16 ctl=(u16)(Pci::ReadConfig(b,s,f,(u8)(cap+0))>>16);bool is64=(ctl&(1u<<7))!=0;u8 vec=(u8)(FirstVector+(sl-slots));u32 addr=0xFEE00000u;Pci::WriteConfig(b,s,f,(u8)(cap+4),addr);if(is64){Pci::WriteConfig(b,s,f,(u8)(cap+8),0);u32 d=Pci::ReadConfig(b,s,f,(u8)(cap+12));d=(d&0xffff0000u)|vec;Pci::WriteConfig(b,s,f,(u8)(cap+12),d);}else{u32 d=Pci::ReadConfig(b,s,f,(u8)(cap+8));d=(d&0xffff0000u)|vec;Pci::WriteConfig(b,s,f,(u8)(cap+8),d);}u32 hdr=Pci::ReadConfig(b,s,f,cap);u16 nctl=(u16)(hdr>>16);nctl|=1u;nctl&=~(7u<<4);Pci::WriteConfig(b,s,f,cap,(hdr&0xffffu)|((u32)nctl<<16));*sl={p.id,r.id,vec,b,s,f,cap,true};r.base=vec;r.length=1;*out=vec;return true;}
void ReleaseProcess(u64 pid){for(auto&x:slots)if(x.active&&x.owner==pid){u32 h=Pci::ReadConfig(x.bus,x.slot,x.function,x.cap);u16 ctl=(u16)(h>>16);ctl&=~1u;Pci::WriteConfig(x.bus,x.slot,x.function,x.cap,(h&0xffffu)|((u32)ctl<<16));x={};}}
extern "C" void DavisExternalIrqDispatch(u64 vector){if(vector<FirstVector||vector>LastVector)return;DriverGateway::DeliverIrq(vector);InterruptController::EndOfInterrupt(PlatformBootstrap::state.controller);}
}
