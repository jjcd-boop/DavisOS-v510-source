#pragma once
#include "MultiAppRuntime.hpp"
#include "LiveProof.hpp"
#include "KernelPlatform.hpp"
namespace Davis::LiveActivation {
enum class Phase:u8 { Cold, Verified, Armed, Running, Cutoff, Passed, Failed };
enum class Failure:u8 { None, RuntimeNotArmed, PlatformNotReady, EndpointInvalid, OwnershipMismatch, SwitchBudgetExceeded, FaultBudgetExceeded };
struct State { Phase phase; Failure failure; u64 firstId; u64 expectedCr3; u64 expectedRsp0; u64 switches; u64 faults; u64 maxSwitches; u64 maxFaults; bool emergencyCutoff; };
void Init(State&,u64 maxSwitches=100000,u64 maxFaults=1);
bool Verify(State&,const MultiAppRuntime::State&,const KernelPlatform::State&);
bool Arm(State&,MultiAppRuntime::State&,KernelPlatform::State&);
bool ObserveSwitch(State&);
bool ObserveFault(State&);
void Cutoff(State&,KernelPlatform::State&);
const char* Diagnostic(const State&,const LiveProof::State&);
}
