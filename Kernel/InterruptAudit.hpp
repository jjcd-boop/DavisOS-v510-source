#pragma once
#include "Idt.hpp"
#include "ApicPlatform.hpp"
#include "InterruptController.hpp"
#include "LocalApicTimer.hpp"
#include "TimerPlatform.hpp"
namespace Davis::InterruptAudit {
enum class Result:u8 { Cold, Ready, BadVector, BadGate, BackendMismatch, TimerMismatch };
struct State { Result result; u8 vector; bool gatePresent; bool kernelOnly; bool backendMatched; bool timerMatched; };
void Init(State&);
bool Verify(State&,const Idt::Table&,u16 kernelCode,const ApicPlatform::State&,const InterruptController::State&,const LocalApicTimer::State&,const TimerPlatform::State&);
}
