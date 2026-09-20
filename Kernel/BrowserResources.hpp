#pragma once
#include "../Base/Types.hpp"
namespace Davis::BrowserResources {
static constexpr u32 MaxResources=24, MaxUrl=384;
enum class Kind:u8{Stylesheet,Image};
enum class Phase:u8{Queued,Ready,Unsupported,Failed};
struct Resource{Kind kind;Phase phase;char url[MaxUrl];u32 bytes;bool valid;};
struct State{Resource items[MaxResources];u32 count;u64 discoveries;bool truncated;};
extern State state;
void Init();
void Discover(const char* html,u32 bytes,const char* baseUrl);
}
