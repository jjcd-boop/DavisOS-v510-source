#pragma once
#include "../Base/Types.hpp"
#include "CpuContext.hpp"
#include "UserMode.hpp"
namespace Davis::ContextSwitch {
using CommitCallback=void(*)(CpuContext::Frame*,u64,u64,void*);
struct Target { CpuContext::Frame* frame; u64 cr3; u64 kernelRsp0; bool sharedKernelMappings; bool valid; CommitCallback onCommit; void* cookie; };
struct State { UserMode::GdtState* gdt; Target pending; u64 commits; u64 rejects; bool hardwareEnabled; };
void Init(State&,UserMode::GdtState&);
bool Stage(State&,CpuContext::Frame*,u64 cr3,u64 kernelRsp0,bool sharedKernelMappings,CommitCallback cb=nullptr,void* cookie=nullptr);
void Cancel(State&);
void SetGlobal(State*);
bool StageGlobal(CpuContext::Frame*,u64 cr3,u64 kernelRsp0,bool sharedKernelMappings);
bool StageGlobalCompletion(CpuContext::Frame*,u64 cr3,u64 kernelRsp0,bool sharedKernelMappings,CommitCallback,void* cookie);
extern "C" CpuContext::Frame* DavisCommitContextSwitch(CpuContext::Frame* fallback);
}
