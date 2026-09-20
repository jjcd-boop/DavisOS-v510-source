#pragma once
#include "TimerPlatform.hpp"
#include "LocalApicTimer.hpp"
#include "InterruptController.hpp"
#include "Preemption.hpp"
#include "PreemptionHealth.hpp"
namespace Davis::TimerActivation {
enum class Result:u8 { NotReady, Armed, Failed };
struct State { Result result; PreemptionHealth::State health; bool irqGateReady; bool controllerReady; bool preemptionReady; };
void Init(State&,u32 targetHz);
bool Validate(const State&,const TimerPlatform::State&,const LocalApicTimer::State&,const InterruptController::State&,const Preemption::State&);
void MarkArmed(State&,u64 nowTsc,u64 interruptCount);
void MarkFailed(State&);
}
