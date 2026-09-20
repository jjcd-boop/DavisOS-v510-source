#include "FaultRecovery.hpp"
namespace Davis::FaultRecovery {
Plan Prepare(Process::Image*f,Scheduler::State&sc,Preemption::State&pr){
 Plan p{};p.faulted=f;if(!f||f->state!=Process::State::Faulted){p.result=Result::NoFault;return p;}if(!sc.ready||!sc.table){p.result=Result::NoScheduler;return p;}
 auto old=sc.currentId;auto cursor=sc.cursor;auto*next=Scheduler::PickNext(sc);sc.cursor=cursor;sc.currentId=old;
 if(!next||next==f){p.result=Result::NoSurvivor;return p;}auto*s=Preemption::FindSlot(pr,next->id);if(!s||!s->valid){p.result=Result::MissingContext;return p;}
 if(!s->cr3||!s->kernelRsp0||!s->sharedKernelMappings||(s->cr3&0xfffull)||(s->kernelRsp0&0xfull)){p.result=Result::UnsafeAddressSpace;return p;}
 p.survivor=next;p.frame=&s->frame;p.cr3=s->cr3;p.kernelRsp0=s->kernelRsp0;p.result=Result::Ready;p.valid=true;return p;
}
void CommitScheduler(const Plan&p,Scheduler::State&sc){if(!p.valid||!p.survivor)return;auto*cur=Scheduler::Current(sc);if(cur&&cur!=p.faulted&&cur!=p.survivor)cur->state=Process::State::Ready;p.survivor->state=Process::State::Running;if(sc.currentId!=p.survivor->id)sc.switches++;sc.currentId=p.survivor->id;sc.sliceTicks=0;}
}
