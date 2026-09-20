#pragma once
#include "BootInfo.hpp"
#include "../Base/Types.hpp"
namespace Davis::ForgePersistentStore {
void Init(const BootInfo&);
bool Available();
i64 Read(u32 appKey,void* dst,u64 bytes);
i64 Write(u32 appKey,const void* src,u64 bytes);
}
