#pragma once
#include "../Base/Types.hpp"
#include "Process.hpp"
#include "Scheduler.hpp"
#include "Preemption.hpp"
namespace Davis::FaultRecovery {
enum class Result:u32 { NoFault, NoScheduler, NoSurvivor, MissingContext, UnsafeAddressSpace, Ready };
struct Plan { Process::Image* faulted; Process::Image* survivor; CpuContext::Frame* frame; u64 cr3; u64 kernelRsp0; Result result; bool valid; };
Plan Prepare(Process::Image* faulted,Scheduler::State&,Preemption::State&);
void CommitScheduler(const Plan&,Scheduler::State&);
}
