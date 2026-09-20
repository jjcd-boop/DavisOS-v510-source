#pragma once
#include "../Base/Types.hpp"
#include "Pci.hpp"
#include "NetworkCore.hpp"
#include "PhysicalMemory.hpp"
namespace Davis::NetworkAdapter {
enum class Backend:u8 { None, IntelE1000, RealtekRtl8168, UnsupportedEthernet };
struct State { Backend backend; bool controllerFound,ready,link; u16 vendor,device; u8 bus,slot,function; u64 tx,rx,txDrops,rxDrops; };
extern State state;
bool Init(const Pci::State&,Memory::PageAllocator&);
bool Send(const void*,u32);
bool Receive(Net::Packet&);
Net::Mac Mac();
const char* BackendName();
}
