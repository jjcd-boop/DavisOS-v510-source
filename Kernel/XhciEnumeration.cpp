#include "XhciEnumeration.hpp"
namespace Davis::Xhci {
static constexpr u32 AddressDeviceType=11, TypeShift=10;
static inline void Barrier(){__asm__ __volatile__("" ::: "memory");}
static Usb::Speed UsbSpeed(u8 s){switch(s){case 1:return Usb::Speed::Full;case 2:return Usb::Speed::Low;case 3:return Usb::Speed::High;case 4:return Usb::Speed::Super;case 5:return Usb::Speed::SuperPlus;default:return Usb::Speed::Unknown;}}
static u16 Ep0Packet(u8 s){switch(s){case 2:return 8;case 3:return 64;case 4:case 5:return 512;default:return 8;}}
static u32 CtxBytes(const State&s){return (s.hcc1&(1u<<2))?64u:32u;}
static void W32(u8*p,u32 off,u32 v){*(u32*)(p+off)=v;}
static bool AllocateSlot(const State&s,Memory::PageAllocator&a,SlotResources&r,u8 slot,u8 port,u8 speed){r={};r.slotId=slot;r.port=port;r.speedId=speed;r.ep0MaxPacket=Ep0Packet(speed);r.ep0Index=0;r.ep0Cycle=true;u32 cs=CtxBytes(s);r.inputContext=Memory::AllocDma(a,cs*33,64);r.deviceContext=Memory::AllocDma(a,cs*32,64);r.ep0Ring=Memory::AllocDma(a,4096,64);if(!r.inputContext.physical||!r.deviceContext.physical||!r.ep0Ring.physical)return false;
 u8*ic=(u8*)r.inputContext.virtualAddress; // Input Control Context: add Slot + EP0.
 W32(ic,4,3u);u8*slotc=ic+cs; // route=0, speed, context entries=1; root-hub port in DWORD1.
 W32(slotc,0,((u32)speed<<20)|(1u<<27));W32(slotc,4,(u32)port<<16);
 u8*ep0=ic+cs*2; // EP0 control endpoint, CErr=3, max packet, dequeue pointer+DCS.
 W32(ep0,4,(3u<<1)|(4u<<3)|((u32)r.ep0MaxPacket<<16));u64 dq=r.ep0Ring.physical|1ull;W32(ep0,8,(u32)dq);W32(ep0,12,(u32)(dq>>32));r.active=true;return true;}
static void FreeSlot(Memory::PageAllocator&a,SlotResources&r){Memory::FreeDma(a,r.hidBuffer);Memory::FreeDma(a,r.hidRing);Memory::FreeDma(a,r.ep0Ring);Memory::FreeDma(a,r.deviceContext);Memory::FreeDma(a,r.inputContext);r={};}
void InitEnumeration(EnumerationState&e){e={};Usb::Init(e.usb);e.ready=true;e.last=EnumResult::Cold;}
void ReleaseEnumeration(EnumerationState&e,Memory::PageAllocator&a){for(auto&r:e.slots)if(r.active)FreeSlot(a,r);InitEnumeration(e);}
EnumResult EnumeratePort(const State&s,const ControllerState&c,DmaState&d,Memory::PageAllocator&a,EnumerationState&e,u8 port,u32 spins){if(!e.ready||!c.running||!d.valid)return e.last=EnumResult::InvalidState;e.attempts++;PortState ps{};auto pr=ResetPort(s,c,port,ps,spins);if(pr==PortResult::NoConnectedPort)return e.last=EnumResult::NoDevice;if(pr!=PortResult::Ok){e.failures++;return e.last=EnumResult::PortResetFailed;}u8 speed=0;for(u32 i=0;i<ps.count;i++)if(ps.ports[i].port==ps.selectedPort){speed=ps.ports[i].speed;break;}CommandCompletion cc{};if(EnableSlot(s,c,d,cc,spins)!=CommandResult::Ok||!cc.slotId||cc.slotId>MaxEnumSlots){e.failures++;return e.last=EnumResult::EnableSlotFailed;}auto&r=e.slots[cc.slotId-1];if(r.active)FreeSlot(a,r);if(!AllocateSlot(s,a,r,cc.slotId,ps.selectedPort,speed)){FreeSlot(a,r);e.failures++;return e.last=EnumResult::NoResources;}((u64*)d.dcbaa.virtualAddress)[cc.slotId]=r.deviceContext.physical;Barrier();u64 cmdPhys=0;if(!QueueCommand(d,r.inputContext.physical,0,(AddressDeviceType<<TypeShift)|((u32)cc.slotId<<24),cmdPhys)||RingDoorbell0(s,c)!=CommandResult::Ok||!PollCommandCompletion(s,d,cmdPhys,cc,spins)||cc.result!=CommandResult::Ok){((u64*)d.dcbaa.virtualAddress)[r.slotId]=0;FreeSlot(a,r);e.failures++;return e.last=EnumResult::AddressFailed;}auto*dev=Usb::Attach(e.usb,r.port,UsbSpeed(r.speedId));if(dev){dev->slotId=r.slotId;dev->address=r.slotId;dev->state=Usb::DeviceState::Addressed;}e.addressed++;return e.last=EnumResult::Ok;}
EnumResult EnumerateFirst(const State&s,const ControllerState&c,DmaState&d,Memory::PageAllocator&a,EnumerationState&e,u32 spins){PortState ps{};ScanPorts(s,c,ps);if(ps.result!=PortResult::Ok)return e.last=EnumResult::InvalidState;for(u32 i=0;i<ps.count;i++)if(ps.ports[i].connected)return EnumeratePort(s,c,d,a,e,ps.ports[i].port,spins);return e.last=EnumResult::NoDevice;}
}
