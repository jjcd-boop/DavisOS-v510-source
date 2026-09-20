#include "Preemption.hpp"
#include "TimerFrame.hpp"
#include "ContextSwitch.hpp"
#include "RuntimeCurrent.hpp"
#include "LiveTrace.hpp"
#include "StackRuntime.hpp"
#include "CommitGuard.hpp"
#include "SystemTasks.hpp"
#include "TrapExit.hpp"
namespace Davis::Preemption {
static State* g=nullptr;
void Init(State&s,Scheduler::State&sc){s={};s.scheduler=&sc;}
void SetGlobal(State*s){g=s;}
bool SaveGlobal(u64 id,const CpuContext::Frame&f){return g?Save(*g,id,f):false;}
Slot* FindSlot(State&s,u64 id){for(auto&x:s.slot)if(x.valid&&x.processId==id)return &x;return nullptr;}
CpuContext::Frame* Find(State&s,u64 id){auto*x=FindSlot(s,id);return x?&x->frame:nullptr;}
bool SaveExecution(State&s,u64 id,const CpuContext::Frame&f,u64 cr3,u64 rsp0){if(!id||!CpuContext::CanonicalRip(f)||!cr3||!rsp0||(cr3&0xfffull)||(rsp0&0xfull))return false;for(auto&x:s.slot)if((x.valid&&x.processId==id)||!x.valid){bool preserve=x.valid&&x.processId==id&&x.cr3==cr3&&x.sharedKernelMappings;x.processId=id;x.frame=f;x.cr3=cr3;x.kernelRsp0=rsp0;x.sharedKernelMappings=preserve;x.valid=true;return true;}return false;}
bool Save(State&s,u64 id,const CpuContext::Frame&f){auto*x=FindSlot(s,id);return x?SaveExecution(s,id,f,x->cr3,x->kernelRsp0):false;}
bool BindAddressSpace(State&s,u64 id,u64 cr3,u64 rsp0,bool shared){if(!id||!cr3||!rsp0||(cr3&0xfffull)||(rsp0&0xfull))return false;auto*x=FindSlot(s,id);if(!x){for(auto&v:s.slot)if(!v.valid){v={};v.valid=true;v.processId=id;x=&v;break;}}if(!x)return false;x->cr3=cr3;x->kernelRsp0=rsp0;x->sharedKernelMappings=shared;return true;}
void AdoptActive(State&s,u64 cr3,u64 rsp0){s.activeCr3=cr3;s.activeKernelRsp0=rsp0;}
static void CommitTimer(CpuContext::Frame*frame,u64 cr3,u64 rsp0,void*cookie){
 auto*s=(State*)cookie;if(!s||!s->pendingTick.valid||!s->scheduler){if(s)s->commitFailures++;return;}
 auto tx=s->pendingTick;s->pendingTick={};if(!Scheduler::CommitTick(*s->scheduler,tx.plan)){s->commitFailures++;return;}
 s->activeCr3=cr3;s->activeKernelRsp0=rsp0;RuntimeCurrent::Set(tx.next);s->contextSwitches++;
 LiveTrace::Record(LiveTrace::Kind::SwitchCommit,tx.next?tx.next->id:0,cr3,rsp0,0,frame?frame->rip:0);
}
CpuContext::Frame* OnTimer(State&s,CpuContext::Frame&f){
 s.timerInterrupts++;auto view=TimerFrame::Normalize(f);if(!view.safeToPreempt||!s.scheduler)return &f;
 // A system slice is requested from the timer independently of application Yield().
 // TimerEntry abandons the selected user frame only after EOI/context commit.
 if(SystemTasks::OnTimerTick())TrapExit::RequestSystemSlice();
 auto*cur=Scheduler::Current(*s.scheduler);u64 old=cur?cur->id:0;auto*oldSlot=old?FindSlot(s,old):nullptr;if(oldSlot)oldSlot->frame=f;
 auto plan=Scheduler::PlanTick(*s.scheduler);if(!plan.valid)return &f;
 if(!s.enabled||!plan.switchRequired){Scheduler::CommitTick(*s.scheduler,plan);return &f;}
 auto*next=plan.next;auto*n=next?FindSlot(s,next->id):nullptr;
 if(!n||!n->cr3||!n->kernelRsp0||!s.activeCr3){s.addressSpaceBlocks++;return &f;}
 if(n->cr3!=s.activeCr3&&!n->sharedKernelMappings){s.addressSpaceBlocks++;return &f;}
 s.pendingTick={plan,n->cr3,n->kernelRsp0,next,true};
 if(!ContextSwitch::StageGlobalCompletion(&n->frame,n->cr3,n->kernelRsp0,n->sharedKernelMappings||n->cr3==s.activeCr3,CommitTimer,&s)){s.pendingTick={};s.addressSpaceBlocks++;return &f;}
 // No scheduler/current-process mutation occurs here. The assembly commit owns it.
 return &n->frame;
}
extern "C" CpuContext::Frame* DavisTimerDispatch(CpuContext::Frame*f){if(!g||!f)return f;auto*p=RuntimeCurrent::Get();if(p&&!StackRuntime::Validate(p->id,(u64)(uptr)f,StackRuntime::EntryKind::Timer))CommitGuard::DavisEmergencyCommitHalt();return OnTimer(*g,*f);}
}
