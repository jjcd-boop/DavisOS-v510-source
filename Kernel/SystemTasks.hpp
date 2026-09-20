#pragma once
#include "../Base/Types.hpp"
namespace Davis::SystemTasks {
enum class Kind:u32 { Desktop=0, Storage=1, Network=2, Audio=3, Count=4 };
struct Task {
 Kind kind; u32 periodTicks; u32 elapsedTicks; u32 priority; u32 budgetPerWindow;
 u32 budgetRemaining; u64 requests; u64 runs; u64 deferred; u64 starvationTicks; u64 maxStarvationTicks; bool enabled;
};
struct State { Task tasks[(u32)Kind::Count]; u64 ticks; u64 dispatches; u64 budgetDeferrals; bool ready; };
extern State state;
void Init(u32 desktopPeriodTicks=4);
void Configure(Kind kind,u32 periodTicks,u32 priority,u32 budgetPerWindow,bool enabled);
bool OnTimerTick();
bool HasPending();
bool PickNext(Kind& kind);
void NoteRun(Kind kind);
const Task* Get(Kind kind);
}
