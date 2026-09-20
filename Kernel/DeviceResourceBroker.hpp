#pragma once
#include "../Base/Types.hpp"
#include "Process.hpp"
namespace Davis::DeviceResourceBroker {
enum class Type:u32 { None=0, Mmio=1, IoPort=2, Irq=3, DmaWindow=4, Msi=5 };
enum Rights:u32 { RightInspect=1u<<0, RightMap=1u<<1, RightSignal=1u<<2, RightDma=1u<<3 };
struct Resource { u64 id; Type type; u64 base; u64 length; u32 rights; u64 deviceId; u64 ownerProcessId; bool active; };
static constexpr usize MaxResources=128;
struct State { Resource resources[MaxResources]; u64 nextId; bool ready; };
extern State state;
void Init();
// Kernel/trusted platform code registers resources. Ring-3 code cannot create them.
u64 Register(Type type,u64 deviceId,u64 base,u64 length,u32 rights);
const Resource* Find(u64 id);
Resource* FindMutable(u64 id);
// Claims are exclusive and only available to authenticated Driver-domain processes.
bool Claim(const Process::Image&,u64 resourceId,u32 requestedRights);
bool Release(const Process::Image&,u64 resourceId);
void RevokeProcess(u64 ownerProcessId);
bool OwnedBy(u64 resourceId,u64 ownerProcessId,u32 requiredRights);
}
