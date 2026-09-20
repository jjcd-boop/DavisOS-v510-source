#include "Xhci.hpp"
#include "../Arch/x86_64/Io.hpp"
namespace Davis::Xhci {
static u32 PciRead(u8 b,u8 d,u8 f,u8 off){
 u32 a=0x80000000u|((u32)b<<16)|((u32)d<<11)|((u32)f<<8)|(off&0xFC);
 Io::Out32(0xCF8,a); return Io::In32(0xCFC);
}
[[maybe_unused]] static void PciWrite(u8 b,u8 d,u8 f,u8 off,u32 v){
 u32 a=0x80000000u|((u32)b<<16)|((u32)d<<11)|((u32)f<<8)|(off&0xFC);
 Io::Out32(0xCF8,a); Io::Out32(0xCFC,v);
}
void Init(State&c){c={};}
bool Discover(State&c,const Pci::State&p){
 c={}; if(!p.xhciFound)return false; auto d=p.xhci; c.discovered=true;c.bus=d.bus;c.device=d.slot;c.function=d.function;
 u32 lo=PciRead(d.bus,d.slot,d.function,0x10),hi=PciRead(d.bus,d.slot,d.function,0x14);if(lo&1)return false;
 c.mmio=(u64)(lo&~0xFULL);if(((lo>>1)&3)==2)c.mmio|=((u64)hi<<32);return c.mmio!=0;
}
bool ReadCapabilities(State&c){
 if(!c.discovered||!c.mmio)return false;
 volatile u8* p=(volatile u8*)(uptr)c.mmio;
 c.capLength=p[0]; c.version=*(volatile u16*)(p+2); c.hcs1=*(volatile u32*)(p+4); c.hcs2=*(volatile u32*)(p+8);
 c.hcc1=*(volatile u32*)(p+0x10); c.dboff=*(volatile u32*)(p+0x14); c.rtsoff=*(volatile u32*)(p+0x18);
 c.maxSlots=(u8)(c.hcs1&0xFF); c.maxPorts=(u8)((c.hcs1>>24)&0xFF);
 c.capabilitiesValid=c.capLength>=0x20&&c.maxPorts>0; return c.capabilitiesValid;
}
bool RequestFirmwareHandoff(State&c,u32 spins){
 if(!c.capabilitiesValid)return false;
 u32 xecp=((c.hcc1>>16)&0xFFFF)*4; if(!xecp)return true;
 volatile u8*base=(volatile u8*)(uptr)c.mmio;
 for(u32 guard=0;xecp&&guard<64;guard++){
  volatile u32*cap=(volatile u32*)(base+xecp);u32 v=*cap;u8 id=(u8)(v&0xFF),next=(u8)((v>>8)&0xFF);
  if(id==1){c.legacySupportFound=true;volatile u32*leg=cap;
   u32 lv=*leg;c.firmwareOwned=(lv&(1u<<16))!=0;*leg=lv|(1u<<24);
   while(c.firmwareOwned&&spins--){lv=*leg;c.firmwareOwned=(lv&(1u<<16))!=0;__asm__ __volatile__("pause");}
   if(c.firmwareOwned)return false;
   // xHCI legacy control/status is the next dword. Once OS ownership is ours,
   // disable firmware SMI generation so firmware cannot race the native driver.
   volatile u32*ctl=leg+1;u32 cv=*ctl;cv&=~0x0000E00Fu;*ctl=cv;
   return true;
  }
  if(!next)break; xecp+=((u32)next)*4;
 }
 return true;
}
}
