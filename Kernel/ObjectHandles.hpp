#pragma once
#include "../Base/Types.hpp"
namespace Davis::ObjectHandles {
enum class Type:u32 { None=0, Window=1, File=2, IpcEndpoint=3, DeviceResource=4 };
struct Entry { u64 token; u64 ownerProcessId; Type type; u64 objectId; u32 rights; bool active; };
static constexpr usize MaxHandles=256;
static constexpr u32 RightRead=1u<<0, RightWrite=1u<<1, RightPresent=1u<<2, RightClose=1u<<3, RightSend=1u<<4, RightReceive=1u<<5, RightDevice=1u<<6;
struct Table { Entry entries[MaxHandles]; u64 nextNonce; };
void Init(Table&);
u64 Create(Table&,u64 owner,Type,u64 objectId,u32 rights);
const Entry* Resolve(const Table&,u64 owner,u64 token,Type expected,u32 requiredRights);
bool Close(Table&,u64 owner,u64 token);
void RevokeProcess(Table&,u64 owner);
}
