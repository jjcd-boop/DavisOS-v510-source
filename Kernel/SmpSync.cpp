#include "SmpSync.hpp"
namespace Davis::SmpSync {
static constexpr u32 UnlockedOwner=~0u;
void Init(SpinLock&l){l.value=0;l.owner=UnlockedOwner;l.acquisitions=0;l.contentions=0;}
bool TryLock(SpinLock&l,u32 cpu){
 if(__sync_lock_test_and_set(&l.value,1u)!=0){__sync_fetch_and_add(&l.contentions,1ull);return false;}
 l.owner=cpu;__sync_fetch_and_add(&l.acquisitions,1ull);__sync_synchronize();return true;
}
void Lock(SpinLock&l,u32 cpu){while(!TryLock(l,cpu)){while(l.value)__asm__ volatile("pause");}}
bool Unlock(SpinLock&l,u32 cpu){if(!l.value||l.owner!=cpu)return false;__sync_synchronize();l.owner=UnlockedOwner;__sync_lock_release(&l.value);return true;}
bool IsLocked(const SpinLock&l){return l.value!=0;}
u32 Owner(const SpinLock&l){return l.owner;}
}
