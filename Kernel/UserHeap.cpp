#include "UserHeap.hpp"
#include "KernelMemory.hpp"
#include "PageTables.hpp"
namespace Davis::UserHeap {
State state{};
void Init(){state={};state.nextVa=0x0000000060000000ull;}
static u64 align(u64 v,u64 a){return (v+a-1)&~(a-1);}
u64 Allocate(const Process::Image&p,u64 bytes){if(!p.id||!p.addressSpaceCr3||!bytes||!KernelMemory::ready)return 0;if(bytes>~0ull-4095)return 0;u64 pages=(bytes+4095)/4096;usize slot=MaxAllocations;for(usize i=0;i<MaxAllocations;i++)if(!state.allocations[i].active){slot=i;break;}if(slot==MaxAllocations)return 0;u64 pa=Memory::AllocPages(KernelMemory::allocator,pages,1);if(!pa)return 0;for(u64 i=0;i<pages*4096;i++)((u8*)(uptr)pa)[i]=0;u64 va=align(state.nextVa,4096);if(va>0x00007ffffffff000ull-pages*4096){Memory::FreePages(KernelMemory::allocator,pa,pages);return 0;}Paging::Space sp{p.addressSpaceCr3,0,true};Vm::Region r{va,pa,pages,Vm::Present|Vm::User|Vm::Writable|Vm::NoExecute};if(Paging::MapRegion(sp,KernelMemory::allocator,r)!=Paging::Result::Ok){Memory::FreePages(KernelMemory::allocator,pa,pages);return 0;}state.allocations[slot]={p.id,va,pa,pages,p.addressSpaceCr3,true};state.nextVa=va+pages*4096+4096;return va;}
bool Free(const Process::Image&p,u64 va){for(auto&a:state.allocations)if(a.active&&a.owner==p.id&&a.va==va){Paging::Space sp{p.addressSpaceCr3,0,true};Paging::UnmapRegion(sp,a.va,a.pages,true);Memory::FreePages(KernelMemory::allocator,a.pa,a.pages);a={};return true;}return false;}
void RevokeProcess(u64 owner){for(auto&a:state.allocations)if(a.active&&a.owner==owner){Paging::Space sp{a.cr3,a.pages,true};Paging::UnmapRegion(sp,a.va,a.pages,true);
 Memory::FreePages(KernelMemory::allocator,a.pa,a.pages);a={};}}
}
