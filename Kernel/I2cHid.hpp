#pragma once
#include "../Base/Types.hpp"
#include "BootInfo.hpp"
#include "AmlLite.hpp"
#include "Pci.hpp"
namespace Davis::I2cHid {
enum class Stage:u8 { Disabled, NoController, NoAcpiDevice, CandidateFound, MmioInvalid, ControllerReady, HidDescriptorReady, Active, Fault };
struct State { Stage stage; bool controllerFound,acpiCandidate,dwVerified,hidDescriptor; u16 pciVendor,pciDevice,targetAddress,hidDescReg,maxInput; u8 pciBus,pciSlot,pciFunction,barIndex; u32 componentType,candidatesChecked; char hid[16]; u64 reports,transactions,errors; i32 dx,dy; bool left; uptr mmio; };
extern State state;
void Init(const BootInfo&);
bool Poll(i32&dx,i32&dy,bool&left);
const char* StageName(Stage);
}
