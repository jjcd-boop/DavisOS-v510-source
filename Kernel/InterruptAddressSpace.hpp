#pragma once
#include "DeviceMappings.hpp"
#include "ApicPlatform.hpp"
namespace Davis::InterruptAddressSpace {
struct Contract { u64 lapicBase; bool lapicMmioRequired; bool installed; };
Contract FromApic(const ApicPlatform::State&);
bool Install(Contract&,Paging::Space&,Memory::PageAllocator&);
bool Verify(const Contract&,const Paging::Space&);
}
