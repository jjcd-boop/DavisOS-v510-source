#pragma once
#include "CpuContext.hpp"
#include "Scheduler.hpp"
#include "Preemption.hpp"
namespace Davis::ExceptionRecovery {
struct State { Scheduler::State* scheduler; Preemption::State* preemption; u64 attempts; u64 recoveries; u64 rejects; bool configured; };
extern State state;
void Configure(Scheduler::State&,Preemption::State&);
void Clear();
CpuContext::Frame* Select(CpuContext::Frame* faultFrame);
extern "C" CpuContext::Frame* DavisExceptionSelectReturn(CpuContext::Frame* faultFrame);
}
