#pragma once
#include "UserMode.hpp"
#include "Idt.hpp"
#include "Scheduler.hpp"
#include "Preemption.hpp"
#include "ContextSwitch.hpp"
#include "InterruptController.hpp"
#include "LocalApicTimer.hpp"
#include "TimerPlatform.hpp"
#include "TimerActivation.hpp"
#include "TimerRuntime.hpp"
#include "TimeReference.hpp"
namespace Davis::KernelPlatform {
enum class Phase:u8 { Cold, CpuOwned, IdtReady, SchedulerReady, ApicReady, ClockReady, TimerReady, Ready, Failed };
struct State {
 Phase phase; UserMode::GdtState* gdt; Idt::Table* idt; Scheduler::State* scheduler;
 Preemption::State* preemption; InterruptController::State* controller; LocalApicTimer::State* timer;
 TimerPlatform::State timerPlatform; TimerActivation::State activation; TimerRuntime::State runtime;
 TimeReference::State time; ContextSwitch::State contextSwitch; u32 targetHz; u8 timerVector; bool liveActivationPermitted;
};
void Init(State&,u32 targetHz=250,u8 vector=LocalApicTimer::DefaultVector);
bool BindCore(State&,UserMode::GdtState&,Idt::Table&,Scheduler::State&,Preemption::State&,InterruptController::State&,LocalApicTimer::State&);
bool AcceptApic(State&,const ApicPlatform::State&);
bool AcceptTime(State&,const TimeReference::State&);
bool AcceptCalibration(State&,const TimerCalibration::Result&);
bool ValidateReady(State&);
void PermitLiveActivation(State&,bool);
bool CanActivate(const State&);
}
