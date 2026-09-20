#pragma once
#include "../Base/Types.hpp"
namespace Davis::SystemServices {
static constexpr usize MaxServices=80;
enum ServiceId:u32 { DesktopCompositor=1, FileService=2, InputDriver=3, ForgeRunner=4 };
struct Entry { u32 serviceId; u64 endpointId; u64 owner; bool active; };
struct State { Entry entries[MaxServices]; bool ready; };
extern State state;
void Init();
bool Register(u32 serviceId,u64 owner,u64 endpointId);
u64 Lookup(u32 serviceId);
void UnregisterOwner(u64 owner);
}
