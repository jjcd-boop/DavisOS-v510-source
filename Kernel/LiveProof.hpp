#pragma once
#include "MultiAppRuntime.hpp"
namespace Davis::LiveProof {
enum class Phase:u8 { Cold, Ready, Running, FaultContained, Passed, Failed };
enum class Failure:u8 { None, RuntimeNotArmed, MissingEndpoints, WrongCurrent, FaultNotContained };
struct State { Phase phase; Failure failure; u64 aId,bId; u64 observedSwitches; u64 faultedId; };
void Init(State&);
bool Prepare(State&,const MultiAppRuntime::State&);
bool ObserveSwitch(State&,u64 processId,u64 cr3,u64 rsp0,PreemptionProof::State&);
bool ObserveFault(State&,Process::Image& faulted,Process::Image& survivor,PreemptionProof::State&);
}
