#pragma once
#include "MultiAppRuntime.hpp"
#include "PlatformBootstrap.hpp"
#include "AppCohort.hpp"
namespace Davis::BootAppRunner {
enum class Result:u8 { NotRun, PlatformNotReady, NoGenericApps, TimerActivationFailed, CohortReturned };
struct State { Result result; AppCohort::State cohort; u64 firstProcessId; u64 returnedProcessId; u64 switches; u64 timerInterrupts; u64 dispatches; u64 isolatedTerminations; bool concurrent; };
extern State state;
void RunGeneric(MultiAppRuntime::State&,PlatformBootstrap::State&);
// Execute one bounded user slice only when explicitly launched from the desktop.
bool LaunchKind(MultiAppRuntime::State&,PlatformBootstrap::State&,u32 kind);
}
