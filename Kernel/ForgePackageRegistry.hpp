#pragma once
#include "../Base/Types.hpp"
namespace Davis::ForgePackageRegistry {
static constexpr u32 MaxPackages=12;
#pragma pack(push,1)
struct Package { u32 magic,version,bytes,budget,caps,digest,appKey; char name[40]; u8 code[128]; };
#pragma pack(pop)
void Init();
bool Install(const Package&);bool Read(u32 slot,Package&);bool Remove(u32 slot);u32 Count();
bool Launch(u32 slot); bool FetchLaunch(u64 pid,Package& out);
}
