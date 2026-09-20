#pragma once
#include "Scheduler.hpp"
#include "SmpRuntime.hpp"
#include "SmpSync.hpp"
namespace Davis::SmpScheduler {
struct State { Scheduler::State* scheduler; SmpRuntime::State* runtime; SmpSync::SpinLock gate; u64 dispatches; u64 rejectedTicks; bool ready; };
bool Init(State&,Scheduler::State&,SmpRuntime::State&);
Process::Image* Tick(State&,u32 apicId);
bool Yield(State&,u32 apicId);
bool Validate(const State&);
}
