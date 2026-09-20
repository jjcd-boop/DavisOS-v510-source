#pragma once
#include "../Base/Types.hpp"
#include "TimerActivation.hpp"
#include "TimeReference.hpp"
namespace Davis::TimerRuntime {
enum class Phase:u8 { Cold, Validated, InterruptsLive, Healthy, RolledBack, Failed };
struct State { Phase phase; u64 interruptsBefore; u64 interruptsAfter; bool rollbackPerformed; };
void Init(State&);
bool Validate(State&,const TimerActivation::State&,const TimerPlatform::State&,const LocalApicTimer::State&,const InterruptController::State&,const Preemption::State&,const TimeReference::State&);
bool Activate(State&,TimerActivation::State&,TimerPlatform::State&,LocalApicTimer::State&,Preemption::State&,const TimeReference::State&);
void Rollback(State&,TimerActivation::State&,TimerPlatform::State&,LocalApicTimer::State&);
bool Verify(State&,TimerActivation::State&,TimerPlatform::State&,LocalApicTimer::State&,Preemption::State&,const TimeReference::State&);
void ResetAfterSlice(State&,TimerActivation::State&,TimerPlatform::State&,LocalApicTimer::State&);
}
