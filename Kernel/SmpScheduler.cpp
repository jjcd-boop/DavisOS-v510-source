#include "SmpScheduler.hpp"
namespace Davis::SmpScheduler {
bool Init(State&s,Scheduler::State&sched,SmpRuntime::State&rt){s={};if(!sched.ready||!SmpRuntime::Validate(rt))return false;s.scheduler=&sched;s.runtime=&rt;SmpSync::Init(s.gate);s.ready=true;return true;}
Process::Image* Tick(State&s,u32 apicId){if(!s.ready||!s.scheduler||!s.runtime){s.rejectedTicks++;return nullptr;}auto*c=SmpRuntime::Lookup(*s.runtime,apicId);if(!c||!c->online){s.rejectedTicks++;return nullptr;}SmpSync::Lock(s.gate,c->logicalId);auto*p=Scheduler::OnTick(*s.scheduler);bool ok=SmpRuntime::AccountSchedulerTick(*s.runtime,apicId);if(ok)s.dispatches++;else s.rejectedTicks++;SmpSync::Unlock(s.gate,c->logicalId);return p;}
bool Yield(State&s,u32 apicId){if(!s.ready||!s.scheduler||!s.runtime)return false;auto*c=SmpRuntime::Lookup(*s.runtime,apicId);if(!c||!c->online)return false;SmpSync::Lock(s.gate,c->logicalId);Scheduler::Yield(*s.scheduler);SmpSync::Unlock(s.gate,c->logicalId);return true;}
bool Validate(const State&s){return s.ready&&s.scheduler&&s.runtime&&s.scheduler->ready&&SmpRuntime::Validate(*s.runtime)&&!SmpSync::IsLocked(s.gate);}
}
