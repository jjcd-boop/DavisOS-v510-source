#pragma once
#include "../Base/Types.hpp"
namespace Davis::Vm {
static constexpr u64 PageSize=4096, UserBase=0x0000000040000000ull, UserTop=0x00007FFFFFFFF000ull;
enum MapFlags:u64 { Present=1,Writable=2,User=4,NoExecute=1ull<<63 };
struct Region { u64 virtualBase,physicalBase,pages,flags; };
struct AddressSpace { static constexpr usize MaxRegions=32; Region regions[MaxRegions];usize count;u64 entry,userStackTop; };
void Init(AddressSpace&);bool AddRegion(AddressSpace&,u64 va,u64 pa,u64 pages,u64 flags);bool Validate(const AddressSpace&);
}
