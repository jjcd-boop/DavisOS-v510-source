#pragma once
#include "../Base/Types.hpp"
namespace Davis::GraphicsObjectManager {
static constexpr usize MaxObjects=64;
static constexpr u64 PerProcessQuotaBytes=16ull*1024*1024;
static constexpr u64 GlobalQuotaBytes=64ull*1024*1024;
enum Rights:u32 { Read=1u<<0, Write=1u<<1, Present=1u<<2, Compose=1u<<3 };
struct Object { u64 id,owner,physical,pages,bytes,generation; u32 width,height,stride,format,rights,refs; bool active; };
struct State { Object objects[MaxObjects]; u64 nextId,totalBytes,allocations,frees,denied; };
extern State state;
void Init();
Object* Create(u64 owner,u32 width,u32 height,u32 rights);
Object* Find(u64 owner,u64 id);
const Object* FindAny(u64 id);
bool Resize(u64 owner,u64 id,u32 width,u32 height);
void Release(u64 owner,u64 id);
void RevokeProcess(u64 owner);
u64 BytesForOwner(u64 owner);
}
