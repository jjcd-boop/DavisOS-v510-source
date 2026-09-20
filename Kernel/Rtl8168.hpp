#pragma once
#include "../Base/Types.hpp"
#include "Pci.hpp"
#include "NetworkCore.hpp"
#include "PhysicalMemory.hpp"
#include "Dma.hpp"
namespace Davis::Rtl8168 {
struct Desc { u32 opts1,opts2; u64 address; } __attribute__((packed));
static constexpr u32 RingCount=32;
struct State { bool found,ready,link; Pci::Device pci; u64 mmio; Memory::DmaBuffer txRing,rxRing,txBuffers,rxBuffers; u32 txTail,rxNext; Net::Mac mac; u64 tx,rx,txDrops,rxDrops; };
extern State state;
bool IsSupported(const Pci::Device&);
bool Init(const Pci::State&,Memory::PageAllocator&);
bool Send(const void*,u32);
bool Receive(Net::Packet&);
}
