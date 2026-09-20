#include "ExceptionRecovery.hpp"
#include "FaultRecovery.hpp"
#include "RuntimeCurrent.hpp"
#include "TrapExit.hpp"
#include "ContextSwitch.hpp"
#include "LiveTrace.hpp"
namespace Davis::ExceptionRecovery {
State state{};
struct Pending { FaultRecovery::Plan plan; bool valid; };
static Pending pending{};
void Configure(Scheduler::State&s,Preemption::State&p){state={};pending={};state.scheduler=&s;state.preemption=&p;state.configured=true;}
void Clear(){state={};pending={};}
static void CommitRecovery(CpuContext::Frame*frame,u64 cr3,u64 rsp0,void*){
 if(!pending.valid||!state.scheduler||!state.preemption){state.rejects++;return;}auto plan=pending.plan;pending={};
 FaultRecovery::CommitScheduler(plan,*state.scheduler);Preemption::AdoptActive(*state.preemption,cr3,rsp0);RuntimeCurrent::Set(plan.survivor);TrapExit::Clear();state.recoveries++;
 LiveTrace::Record(LiveTrace::Kind::RecoveryCommit,plan.survivor?plan.survivor->id:0,cr3,rsp0,0,frame?frame->rip:0);
}
CpuContext::Frame* Select(CpuContext::Frame*){
 state.attempts++;if(!state.configured||!state.scheduler||!state.preemption){state.rejects++;return nullptr;}
 auto*f=RuntimeCurrent::Get();if(!f||f->state!=Process::State::Faulted){state.rejects++;return nullptr;}
 auto plan=FaultRecovery::Prepare(f,*state.scheduler,*state.preemption);if(!plan.valid){state.rejects++;return nullptr;}
 pending={plan,true};if(!ContextSwitch::StageGlobalCompletion(plan.frame,plan.cr3,plan.kernelRsp0,true,CommitRecovery,nullptr)){pending={};state.rejects++;return nullptr;}
 // Ownership is finalized only after the assembly-side CR3/TSS commit succeeds.
 return plan.frame;
}
extern "C" CpuContext::Frame* DavisExceptionSelectReturn(CpuContext::Frame*f){return Select(f);}
}
