#pragma once
#include "XhciEnumeration.hpp"
#include "Pci.hpp"
#include "UsbHid.hpp"
namespace Davis::UsbRuntime {
enum class Result:u32 { Cold,NoController,CapabilitiesFailed,DmaFailed,StartFailed,NoDevice,Addressed };
enum class Bringup:u32 { Cold,PciScanned,Discovered,Capabilities,DmaAllocated,StartEntered,Started,Enumerating,Ready };
struct State { Pci::State pci; Xhci::State xhci; Xhci::DmaState dma; Xhci::ControllerState controller; Xhci::EnumerationState enumeration; Result result; Bringup bringup; bool ready; bool ring3Owned; u64 polls,events,reports; u32 connectedPorts,hidDevices,keyboards,mice; u16 lastVendor,lastProduct; u8 lastCompletion,lastSlot,lastDci; };
extern State state;
void Init();
bool RelinquishToRing3();
bool PollHid(UsbHid::Keyboard&,UsbHid::Mouse&,char& key,bool& mouseChanged);
}
