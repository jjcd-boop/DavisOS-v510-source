#include "Rtl8168.hpp"
namespace Davis::Rtl8168 {
State state{};
static volatile u8* B(u32 o){return (volatile u8*)(uptr)(state.mmio+o);} static volatile u16* W(u32 o){return (volatile u16*)(uptr)(state.mmio+o);} static volatile u32* D(u32 o){return (volatile u32*)(uptr)(state.mmio+o);} static void barrier(){__asm__ __volatile__("":::"memory");}
static void release(Memory::PageAllocator&a){if(state.txRing.physical)Memory::FreeDma(a,state.txRing);if(state.rxRing.physical)Memory::FreeDma(a,state.rxRing);if(state.txBuffers.physical)Memory::FreeDma(a,state.txBuffers);if(state.rxBuffers.physical)Memory::FreeDma(a,state.rxBuffers);}
bool IsSupported(const Pci::Device&d){return d.vendor==0x10EC&&(d.device==0x8168||d.device==0x8169||d.device==0x8125);}
bool Init(const Pci::State&p,Memory::PageAllocator&a){state={};for(u32 i=0;i<p.count;i++)if(IsSupported(p.devices[i])){state.pci=p.devices[i];state.found=true;break;}if(!state.found)return false;state.mmio=Pci::MemoryBarAddress(state.pci,0);if(!state.mmio)return false;Pci::EnableMemoryBusMaster(state.pci);
 /* Disable interrupts and reset. The driver is deliberately polling-first for bring-up stability. */ *W(0x3C)=0;*B(0x37)=0x10;for(u32 n=0;n<1000000&&(*B(0x37)&0x10);n++)__asm__ __volatile__("pause");if(*B(0x37)&0x10)return false;
 for(u32 i=0;i<6;i++)state.mac.b[i]=*B(i);
 state.txRing=Memory::AllocDma(a,sizeof(Desc)*RingCount,256);state.rxRing=Memory::AllocDma(a,sizeof(Desc)*RingCount,256);state.txBuffers=Memory::AllocDma(a,2048*RingCount,4096);state.rxBuffers=Memory::AllocDma(a,2048*RingCount,4096);if(!state.txRing.physical||!state.rxRing.physical||!state.txBuffers.physical||!state.rxBuffers.physical){release(a);return false;}
 auto*tx=(Desc*)state.txRing.virtualAddress;auto*rx=(Desc*)state.rxRing.virtualAddress;for(u32 i=0;i<RingCount;i++){tx[i]={};rx[i]={};rx[i].address=state.rxBuffers.physical+(u64)i*2048;rx[i].opts1=(1u<<31)|2048u|(i==RingCount-1?(1u<<30):0);}
 *B(0x50)=0xC0; /* unlock config */
 *D(0x20)=(u32)state.txRing.physical;*D(0x24)=(u32)(state.txRing.physical>>32);*D(0xE4)=(u32)state.rxRing.physical;*D(0xE8)=(u32)(state.rxRing.physical>>32);
 *W(0xDA)=2048;*D(0x44)=(*D(0x44)&~0x3Fu)|(7u<<8);*D(0x40)|=(3u<<8);*B(0xEC)=0x3F;*B(0x37)=0x0C;*B(0x50)=0x00;barrier();state.txTail=state.rxNext=0;state.link=((*B(0x6C))&0x02)!=0;state.ready=true;return true;}
bool Send(const void*f,u32 n){if(!state.ready||!f||!n||n>1518){state.txDrops++;return false;}auto*d=(Desc*)state.txRing.virtualAddress;u32 i=state.txTail;if(d[i].opts1&(1u<<31)){state.txDrops++;return false;}u8*dst=(u8*)state.txBuffers.virtualAddress+i*2048;for(u32 j=0;j<n;j++)dst[j]=((const u8*)f)[j];d[i].address=state.txBuffers.physical+(u64)i*2048;barrier();d[i].opts2=0;d[i].opts1=(1u<<31)|(1u<<29)|(1u<<28)|n|(i==RingCount-1?(1u<<30):0);barrier();*B(0x38)=0x40;state.txTail=(i+1)%RingCount;state.tx++;return true;}
bool Receive(Net::Packet&o){if(!state.ready)return false;auto*d=(Desc*)state.rxRing.virtualAddress;u32 i=state.rxNext;u32 q=d[i].opts1;if(q&(1u<<31))return false;u32 n=q&0x3FFFu;if(n>=4)n-=4;if(n>Net::MaxFrame)n=0;o.bytes=n;u8*src=(u8*)state.rxBuffers.virtualAddress+i*2048;for(u32 j=0;j<n;j++)o.data[j]=src[j];barrier();d[i].opts1=(1u<<31)|2048u|(i==RingCount-1?(1u<<30):0);state.rxNext=(i+1)%RingCount;if(n)state.rx++;else state.rxDrops++;return n!=0;}
}
