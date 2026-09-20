#include "SystemTasks.hpp"
namespace Davis::SystemTasks {
State state{};
static Task* task(Kind k){u32 i=(u32)k;return i<(u32)Kind::Count?&state.tasks[i]:nullptr;}
void Configure(Kind k,u32 period,u32 priority,u32 budget,bool enabled){auto*t=task(k);if(!t)return;t->kind=k;t->periodTicks=period?period:1;t->priority=priority;t->budgetPerWindow=budget?budget:1;t->budgetRemaining=t->budgetPerWindow;t->enabled=enabled;}
void Init(u32 desktopPeriodTicks){state={};Configure(Kind::Desktop,desktopPeriodTicks,100,1,true);Configure(Kind::Storage,8,80,1,true);Configure(Kind::Network,4,70,1,true);Configure(Kind::Audio,2,90,1,true);state.ready=true;}
bool OnTimerTick(){if(!state.ready)return false;state.ticks++;bool pending=false;for(u32 i=0;i<(u32)Kind::Count;i++){auto&t=state.tasks[i];if(!t.enabled)continue;if(++t.elapsedTicks>=t.periodTicks){t.elapsedTicks=0;t.budgetRemaining=t.budgetPerWindow;t.requests++;}if(t.requests>t.runs){t.starvationTicks++;if(t.starvationTicks>t.maxStarvationTicks)t.maxStarvationTicks=t.starvationTicks;pending=true;}}return pending;}
bool HasPending(){if(!state.ready)return false;for(u32 i=0;i<(u32)Kind::Count;i++){auto&t=state.tasks[i];if(t.enabled&&t.requests>t.runs)return true;}return false;}
bool PickNext(Kind& out){if(!state.ready)return false;Task*best=nullptr;for(u32 i=0;i<(u32)Kind::Count;i++){auto&t=state.tasks[i];if(!t.enabled||t.requests<=t.runs)continue;if(!t.budgetRemaining){t.deferred++;state.budgetDeferrals++;continue;}if(!best||t.priority>best->priority||(t.priority==best->priority&&t.starvationTicks>best->starvationTicks))best=&t;}if(!best)return false;out=best->kind;return true;}
void NoteRun(Kind k){auto*t=task(k);if(!t||!t->enabled)return;if(t->runs<t->requests)t->runs++;if(t->budgetRemaining)t->budgetRemaining--;t->starvationTicks=0;state.dispatches++;}
const Task* Get(Kind k){return task(k);}
}
