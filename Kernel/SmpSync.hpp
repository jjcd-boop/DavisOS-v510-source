#pragma once
#include "../Base/Types.hpp"
namespace Davis::SmpSync {
struct SpinLock { volatile u32 value; u32 owner; u64 acquisitions; u64 contentions; };
void Init(SpinLock&);
bool TryLock(SpinLock&,u32 cpuId);
void Lock(SpinLock&,u32 cpuId);
bool Unlock(SpinLock&,u32 cpuId);
bool IsLocked(const SpinLock&);
u32 Owner(const SpinLock&);
}
