#include "KernelMemory.hpp"
namespace Davis::KernelMemory {
Memory::PageAllocator allocator{}; bool ready=false;
void Init(const BootInfo&b){Memory::Init(allocator,b);ready=allocator.stats.freePages!=0;}
const Memory::Stats& Stats(){return allocator.stats;}
}
