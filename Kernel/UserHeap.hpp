#pragma once
#include "../Base/Types.hpp"
#include "Process.hpp"
namespace Davis::UserHeap {
static constexpr usize MaxAllocations=128;
struct Allocation{u64 owner,va,pa,pages,cr3;bool active;};
struct State{Allocation allocations[MaxAllocations];u64 nextVa;};
extern State state;
void Init();
u64 Allocate(const Process::Image&,u64 bytes);
bool Free(const Process::Image&,u64 va);
void RevokeProcess(u64 owner);
}
