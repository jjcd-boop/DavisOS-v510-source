#pragma once
#include "../Base/Types.hpp"
#include "CpuContext.hpp"
#include "Scheduler.hpp"
namespace Davis::Preemption {
static constexpr usize MaxContexts=Process::MaxProcesses;
struct Slot { u64 processId; CpuContext::Frame frame; u64 cr3; u64 kernelRsp0; bool sharedKernelMappings; bool valid; };
struct PendingTick { Scheduler::TickPlan plan; u64 cr3; u64 kernelRsp0; Process::Image* next; bool valid; };
struct State { Scheduler::State* scheduler; Slot slot[MaxContexts]; u64 timerInterrupts; u64 contextSwitches; u64 addressSpaceBlocks; u64 commitFailures; u64 activeCr3; u64 activeKernelRsp0; PendingTick pendingTick; bool enabled; };
void Init(State&,Scheduler::State&);
bool Save(State&,u64 processId,const CpuContext::Frame&);
bool SaveExecution(State&,u64 processId,const CpuContext::Frame&,u64 cr3,u64 kernelRsp0);
bool BindAddressSpace(State&,u64 processId,u64 cr3,u64 kernelRsp0,bool sharedKernelMappings=false);
CpuContext::Frame* Find(State&,u64 processId);
Slot* FindSlot(State&,u64 processId);
CpuContext::Frame* OnTimer(State&,CpuContext::Frame& interrupted);
void AdoptActive(State&,u64 cr3,u64 kernelRsp0);
void SetGlobal(State*);
bool SaveGlobal(u64 processId,const CpuContext::Frame&);
extern "C" CpuContext::Frame* DavisTimerDispatch(CpuContext::Frame*);
}
