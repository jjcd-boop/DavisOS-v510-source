#pragma once
#include "../Base/Types.hpp"
#include "BootInfo.hpp"
namespace Davis::Memory {
static constexpr u64 PageSize=4096;
struct Stats { u64 totalPages,freePages,reservedPages,allocatedPages; };
struct PageAllocator {
 static constexpr usize MaxRanges=1024;
 struct Range { u64 base,pages; };
 Range freeRanges[MaxRanges]; usize rangeCount; Stats stats;
};
void Init(PageAllocator&, const BootInfo&);
void Reserve(PageAllocator&,u64 base,u64 bytes);
u64 AllocPages(PageAllocator&,u64 pages,u64 alignmentPages=1);
bool FreePages(PageAllocator&,u64 base,u64 pages);
bool IsPageAligned(u64);
}
