#pragma once
#include "../Base/Types.hpp"
#include "PhysicalMemory.hpp"
#include "VirtualMemory.hpp"
namespace Davis::Paging {
static constexpr u64 P=1ull<<0,W=1ull<<1,U=1ull<<2,NX=1ull<<63,AddrMask=0x000ffffffffff000ull;
struct Space { u64 pml4Physical; u64 mappedPages; bool ready; };
enum class Result:u32 { Ok,InvalidAddress,OutOfMemory,AlreadyMapped,NotMapped };
void Init(Space&);
Result Create(Space&,Memory::PageAllocator&);
Result Map4K(Space&,Memory::PageAllocator&,u64 va,u64 pa,u64 flags);
Result MapKernel4K(Space&,Memory::PageAllocator&,u64 va,u64 pa,u64 flags);
// Supervisor-only mapping at an explicitly selected VA, including lower-half MMIO aliases.
// Unlike Map4K this never sets U and therefore cannot be accessed from CPL3.
Result MapSupervisor4K(Space&,Memory::PageAllocator&,u64 va,u64 pa,u64 flags);
Result MapRegion(Space&,Memory::PageAllocator&,const Vm::Region&);
Result Unmap4K(Space&,u64 va,bool flushIfActive=true);
Result UnmapRegion(Space&,u64 va,u64 pages,bool flushIfActive=true);
bool Translate(const Space&,u64 va,u64* physicalOut,u64* entryOut=nullptr);
void FlushPage(u64 va);
void FlushSpaceIfActive(const Space&);
}
