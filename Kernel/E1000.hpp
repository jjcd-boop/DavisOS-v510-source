#pragma once
#include "../Base/Types.hpp"
#include "Pci.hpp"
#include "NetworkCore.hpp"
#include "PhysicalMemory.hpp"
#include "Dma.hpp"
namespace Davis::E1000 {
struct TxDesc { u64 address; u16 length; u8 cso,cmd; u8 status,css; u16 special; } __attribute__((packed));
struct RxDesc { u64 address; u16 length,checksum; u8 status,errors; u16 special; } __attribute__((packed));
static constexpr u32 RingCount=32;
struct State { bool found,ready,link; Pci::Device pci; u64 mmio; Memory::DmaBuffer txRing,rxRing,txBuffers,rxBuffers; u32 txTail,rxNext; Net::Mac mac; u64 tx,rx,txDrops,rxDrops; };
extern State state;
bool IsSupported(const Pci::Device& d);
bool Init(const Pci::State& pci,Memory::PageAllocator& allocator);
bool Send(const void* frame,u32 bytes);
bool Receive(Net::Packet& out);
}
