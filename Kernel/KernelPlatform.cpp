#include "KernelPlatform.hpp"
extern "C" void DavisTimerEntry();
namespace Davis::KernelPlatform {
void Init(State&s,u32 hz,u8 vector){s={};s.phase=Phase::Cold;s.targetHz=hz;s.timerVector=vector;TimerPlatform::Init(s.timerPlatform,hz,vector);TimerActivation::Init(s.activation,hz);}
bool BindCore(State&s,UserMode::GdtState&g,Idt::Table&i,Scheduler::State&sc,Preemption::State&p,InterruptController::State&ic,LocalApicTimer::State&t){
 if(!UserMode::Validate(g)||!i.ready||!sc.ready||p.scheduler!=&sc){s.phase=Phase::Failed;return false;}
 s.gdt=&g;ContextSwitch::Init(s.contextSwitch,g);ContextSwitch::SetGlobal(&s.contextSwitch);s.idt=&i;s.scheduler=&sc;s.preemption=&p;s.controller=&ic;s.timer=&t;s.phase=Phase::SchedulerReady;return true;
}
bool AcceptApic(State&s,const ApicPlatform::State&a){if(s.phase!=Phase::SchedulerReady||!TimerPlatform::AcceptApic(s.timerPlatform,a)){s.phase=Phase::Failed;return false;}if(!(a.x2Apic?InterruptController::InitX2Apic(*s.controller):InterruptController::InitLocalApic(*s.controller,a.physicalBase))){s.phase=Phase::Failed;return false;}s.phase=Phase::ApicReady;return true;}
bool AcceptTime(State&s,const TimeReference::State&t){if(s.phase!=Phase::ApicReady||!t.usable||!t.invariant||!t.tscHz){s.phase=Phase::Failed;return false;}s.time=t;s.phase=Phase::ClockReady;return true;}
bool AcceptCalibration(State&s,const TimerCalibration::Result&r){if(s.phase!=Phase::ClockReady)return false;if(!TimerPlatform::InstallGate(s.timerPlatform,*s.idt,s.gdt->selectors.kernelCode,DavisTimerEntry)){s.phase=Phase::Failed;return false;}if(!TimerPlatform::AcceptCalibration(s.timerPlatform,r)){s.phase=Phase::Failed;return false;}if(!(s.timerPlatform.apic.x2Apic?LocalApicTimer::ConfigurePeriodicX2(*s.timer,s.timerVector,r.initialCount):LocalApicTimer::ConfigurePeriodic(*s.timer,s.timerPlatform.apic.physicalBase,s.timerVector,r.initialCount))){s.phase=Phase::Failed;return false;}s.phase=Phase::TimerReady;return true;}
bool ValidateReady(State&s){if(s.phase!=Phase::TimerReady||!TimerRuntime::Validate(s.runtime,s.activation,s.timerPlatform,*s.timer,*s.controller,*s.preemption,s.time)){s.phase=Phase::Failed;return false;}s.phase=Phase::Ready;return true;}
void PermitLiveActivation(State&s,bool yes){s.liveActivationPermitted=yes;if(!yes)s.contextSwitch.hardwareEnabled=false;}
bool CanActivate(const State&s){return s.phase==Phase::Ready&&s.liveActivationPermitted&&s.runtime.phase==TimerRuntime::Phase::Validated;}
}
