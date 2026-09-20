#include "TimerActivation.hpp"
namespace Davis::TimerActivation {
void Init(State&s,u32 hz){s={};s.result=Result::NotReady;PreemptionHealth::Init(s.health,hz);}
bool Validate(const State&,const TimerPlatform::State&p,const LocalApicTimer::State&t,const InterruptController::State&ic,const Preemption::State&pre){return TimerPlatform::ReadyToArm(p)&&t.configured&&!t.armed&&LocalApicTimer::Validate(t)&&ic.ready&&(ic.kind==InterruptController::Kind::LocalApic||ic.kind==InterruptController::Kind::X2Apic)&&pre.scheduler;}
void MarkArmed(State&s,u64 now,u64 ints){s.result=Result::Armed;s.irqGateReady=true;s.controllerReady=true;s.preemptionReady=true;PreemptionHealth::Arm(s.health,now,ints);}
void MarkFailed(State&s){s.result=Result::Failed;PreemptionHealth::Fail(s.health);}
}
