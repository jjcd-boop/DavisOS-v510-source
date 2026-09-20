#pragma once
#include "PhysicalMemory.hpp"
namespace Davis::Memory {
struct DmaBuffer { u64 physical; void* virtualAddress; u64 bytes,pages,alignment; };
DmaBuffer AllocDma(PageAllocator&,u64 bytes,u64 alignment=64);
bool FreeDma(PageAllocator&,DmaBuffer&);
void Zero(DmaBuffer&);
}
