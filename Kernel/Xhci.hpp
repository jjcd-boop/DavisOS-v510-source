#pragma once
#include "../Base/Types.hpp"
#include "Pci.hpp"
namespace Davis::Xhci {
struct State {
 bool discovered,capabilitiesValid,legacySupportFound,firmwareOwned;
 u8 bus,device,function,capLength,maxSlots,maxPorts;
 u16 version; u64 mmio; u32 hcs1,hcs2,hcc1,dboff,rtsoff;
};
void Init(State&); bool Discover(State&,const Pci::State&); bool ReadCapabilities(State&); bool RequestFirmwareHandoff(State&,u32 spinLimit=1000000);
}
