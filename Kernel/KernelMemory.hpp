#pragma once
#include "PhysicalMemory.hpp"
namespace Davis::KernelMemory {
extern Memory::PageAllocator allocator;
extern bool ready;
void Init(const BootInfo&);
const Memory::Stats& Stats();
}
