#pragma once
#include "../Base/Types.hpp"
namespace Davis::KernelClock {
struct State { u64 ticks; u64 frequencyHz; bool calibrated; };
void Init(State&,u64 frequencyHz);
void Tick(State&);
u64 Milliseconds(const State&);
}
