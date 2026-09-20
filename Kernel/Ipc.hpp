#pragma once
#include "../Base/Types.hpp"
namespace Davis::Ipc {
static constexpr usize MaxEndpoints=32, MaxMessages=64, MaxPayload=192;
enum Rights:u32 { RightSend=1u<<0, RightReceive=1u<<1 };
struct Message { u64 sender; u32 type; u32 bytes; u8 payload[MaxPayload]; bool active; };
struct Endpoint { u64 id,owner; u32 rights; u32 head,tail,count; bool active; Message queue[8]; };
struct State { Endpoint endpoints[MaxEndpoints]; u64 nextId; };
extern State state;
void Init();
u64 CreateEndpoint(u64 owner,u32 rights);
bool Send(u64 sender,u64 endpointId,u32 type,const void*data,u32 bytes);
i64 Receive(u64 owner,u64 endpointId,u64*sender,u32*type,void*data,u32 capacity);
u64 OwnerOf(u64 endpointId);
void RevokeProcess(u64 owner);
}
