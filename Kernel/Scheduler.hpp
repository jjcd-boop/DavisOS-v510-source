#pragma once
#include "../Base/Types.hpp"
#include "Process.hpp"
namespace Davis::Scheduler {
struct Config { u32 quantumTicks; };
struct State { Process::Table* table; u64 ticks; u64 switches; u64 currentId; usize cursor; u32 sliceTicks; Config config; bool ready; };
struct TickPlan { Process::Image* current; Process::Image* next; usize nextCursor; u32 nextSliceTicks; bool switchRequired; bool valid; };
void Init(State&, Process::Table&, Config cfg={4});
Process::Image* Current(State&);
Process::Image* PickNext(State&);
Process::Image* OnTick(State&);
TickPlan PlanTick(State&);
bool CommitTick(State&,const TickPlan&);
void BlockCurrent(State&);
void Wake(Process::Image&);
void Yield(State&);
}
