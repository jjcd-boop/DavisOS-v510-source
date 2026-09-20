#pragma once
#include "MultiAppRuntime.hpp"
namespace Davis::AppCohort {
struct State { u64 memberIds[MaxBootApps]; usize count; usize runnable; u64 firstId; bool armed; };
void Build(State&,MultiAppRuntime::State&);
Process::Image* FirstRunnable(State&,MultiAppRuntime::State&);
void RetireTerminal(State&,MultiAppRuntime::State&);
void AbortSurvivors(State&,MultiAppRuntime::State&,int code=-125);
}
