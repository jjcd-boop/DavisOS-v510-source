#pragma once
#include "XhciCommands.hpp"
#include "XhciPorts.hpp"
#include "UsbCore.hpp"
namespace Davis::Xhci {
static constexpr u32 MaxEnumSlots=32;
enum class EnumResult:u32 { Cold,Ok,NoDevice,PortResetFailed,EnableSlotFailed,NoResources,AddressFailed,InvalidState };
struct SlotResources { Memory::DmaBuffer inputContext,deviceContext,ep0Ring,hidRing,hidBuffer; u8 slotId,port,speedId; u16 ep0MaxPacket; u32 ep0Index,hidIndex; bool ep0Cycle,hidCycle; u8 hidDci,hidProtocol,hidReportBytes; bool hidReady,hidQueued; bool active; };
struct EnumerationState { Usb::Registry usb; SlotResources slots[MaxEnumSlots]; u64 attempts,addressed,failures; EnumResult last; bool ready; };
void InitEnumeration(EnumerationState&);
void ReleaseEnumeration(EnumerationState&,Memory::PageAllocator&);
EnumResult EnumeratePort(const State&,const ControllerState&,DmaState&,Memory::PageAllocator&,EnumerationState&,u8 port,u32 spins=2000000);
EnumResult EnumerateFirst(const State&,const ControllerState&,DmaState&,Memory::PageAllocator&,EnumerationState&,u32 spins=2000000);
}
