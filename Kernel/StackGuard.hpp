#pragma once
#include "../Base/Types.hpp"
#include "PhysicalMemory.hpp"
#include "PageTables.hpp"
namespace Davis::StackGuard {
static constexpr u64 Canary=0x4456415353544B47ull; // "DVASSTKG"
struct Stack { u64 allocationBase,guardBase,usableBase,top,usablePages; volatile u64* canary; bool ready; };
bool Allocate(Stack&,Memory::PageAllocator&,u64 usablePages);
bool MapInto(const Stack&,Paging::Space&,Memory::PageAllocator&);
bool VerifyMapped(const Stack&,const Paging::Space&);
bool CanaryIntact(const Stack&);
}
