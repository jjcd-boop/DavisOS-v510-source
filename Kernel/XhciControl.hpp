#pragma once
#include "XhciEnumeration.hpp"
namespace Davis::Xhci {
enum class ControlResult:u32 { Ok, InvalidState, NoResources, RingFull, Timeout, TransferError, BadDescriptor, EvaluateFailed };
struct TransferCompletion { u8 completionCode; u8 slotId; u8 endpointId; u32 residual; };
ControlResult GetDescriptor(const State&,const ControllerState&,DmaState&,SlotResources&,Memory::PageAllocator&,u8 descriptorType,u8 descriptorIndex,u16 languageId,void* out,u16 bytes,u32 spins=2000000);
ControlResult ConfigureBootHid(const State&,const ControllerState&,DmaState&,SlotResources&,Memory::PageAllocator&,Usb::Device&,u32 spins=2000000);
ControlResult ReadAndConfigureDescriptors(const State&,const ControllerState&,DmaState&,SlotResources&,Memory::PageAllocator&,Usb::Device&,u32 spins=2000000);
}
