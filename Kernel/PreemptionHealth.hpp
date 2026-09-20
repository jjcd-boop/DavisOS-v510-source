#pragma once
#include "../Base/Types.hpp"
namespace Davis::PreemptionHealth {
enum class Status:u8 { Cold, Armed, Healthy, Stalled, Failed };
struct State {
    Status status;
    u64 armedTsc;
    u64 lastCheckTsc;
    u64 lastInterrupts;
    u64 observedInterrupts;
    u32 expectedHz;
    u32 graceMilliseconds;
    bool activationCommitted;
};
void Init(State&,u32 expectedHz,u32 graceMilliseconds=100);
void Arm(State&,u64 nowTsc,u64 currentInterrupts);
bool Observe(State&,u64 nowTsc,u64 tscHz,u64 currentInterrupts);
bool Healthy(const State&);
void Fail(State&);
}
