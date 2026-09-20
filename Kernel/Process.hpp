#pragma once
#include "../Base/Types.hpp"
#include "Dxe.hpp"
#include "SecurityDomain.hpp"
namespace Davis::Process {
enum class State:u32 { Empty,Created,Ready,Running,Blocked,Exited,Faulted };
struct Image {
 u64 id; State state; const Dxe::Header* header; const u8* fileImage;
 u64 fileBytes; u64 entryVirtual; u64 userStackTop; int exitCode; u64 lastFaultVector; u64 launchAttempts;
 u64 requestedCapabilities; u64 grantedCapabilities; u64 addressSpaceCr3; SecurityDomain::Kind domain; bool dxe2;
};
static constexpr usize MaxProcesses=64;
struct Table { Image process[MaxProcesses]; u64 nextId; };
void Init(Table&);
Image* CreateValidated(Table&,const void* dxe,u64 bytes,u64 mappedBase,u64 userStackTop);
void MarkExited(Image&,int code);
}
