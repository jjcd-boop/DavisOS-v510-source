#pragma once
#include "../Base/Types.hpp"
namespace Davis::Pci {
struct Device{u8 bus,slot,function;u16 vendor,device;u8 classCode,subclass,progIf;u32 bar0;};
struct State{Device devices[128];u32 count;
 bool xhciFound;Device xhci;
 bool nvmeFound;Device nvme;
 bool ahciFound;Device ahci;
 bool raidFound;Device raid;
 bool intelVmdCandidateFound;Device intelVmdCandidate;
 u32 storageControllerCount;
 bool hdaFound;Device hda;
 bool i2cFound;Device i2c; u32 i2cCount;
 bool ethernetFound;Device ethernet;
 bool networkFound;Device network;
 bool displayFound;Device display;
};
u32 ReadConfig(u8 bus,u8 slot,u8 function,u8 offset);
void WriteConfig(u8 bus,u8 slot,u8 function,u8 offset,u32 value);
void EnableMemoryBusMaster(const Device&);
// Decode a memory BAR directly from PCI config space, including 64-bit BARs.
// Returns 0 for I/O BARs, invalid indices, or an unassigned BAR.
u64 MemoryBarAddress(const Device&,u8 index=0);
void Scan(State&);
}
