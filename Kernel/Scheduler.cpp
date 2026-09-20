#include "Scheduler.hpp"
namespace Davis::Scheduler {
static bool runnable(const Process::Image&p){return p.state==Process::State::Ready||p.state==Process::State::Running;}
void Init(State&s,Process::Table&t,Config c){s={};s.table=&t;s.config.quantumTicks=c.quantumTicks?c.quantumTicks:1;s.ready=true;}
Process::Image* Current(State&s){if(!s.table||!s.currentId)return nullptr;for(usize i=0;i<Process::MaxProcesses;i++)if(s.table->process[i].id==s.currentId&&s.table->process[i].state==Process::State::Running)return &s.table->process[i];return nullptr;}
Process::Image* PickNext(State&s){if(!s.ready||!s.table)return nullptr;for(usize n=0;n<Process::MaxProcesses;n++){usize i=(s.cursor+n)%Process::MaxProcesses;auto&p=s.table->process[i];if(runnable(p)){s.cursor=(i+1)%Process::MaxProcesses;return &p;}}return nullptr;}
TickPlan PlanTick(State&s){
 TickPlan p{};if(!s.ready||!s.table)return p;p.current=Current(s);p.nextSliceTicks=s.sliceTicks;
 if(p.current&&p.nextSliceTicks+1<s.config.quantumTicks){p.next=p.current;p.nextSliceTicks++;p.nextCursor=s.cursor;p.valid=true;return p;}
 for(usize n=0;n<Process::MaxProcesses;n++){usize i=(s.cursor+n)%Process::MaxProcesses;auto&candidate=s.table->process[i];if(runnable(candidate)){p.next=&candidate;p.nextCursor=(i+1)%Process::MaxProcesses;break;}}
 p.nextSliceTicks=0;p.switchRequired=p.next&&(!p.current||p.next->id!=p.current->id);p.valid=true;return p;
}
bool CommitTick(State&s,const TickPlan&p){
 if(!p.valid)return false;s.ticks++;s.cursor=p.nextCursor;s.sliceTicks=p.nextSliceTicks;
 if(!p.switchRequired)return true;
 if(p.current&&p.current->state==Process::State::Running)p.current->state=Process::State::Ready;
 if(p.next){p.next->state=Process::State::Running;if(s.currentId!=p.next->id)s.switches++;s.currentId=p.next->id;}else s.currentId=0;
 return true;
}
Process::Image* OnTick(State&s){auto p=PlanTick(s);if(!CommitTick(s,p))return nullptr;return p.next;}
void BlockCurrent(State&s){auto*p=Current(s);if(p)p->state=Process::State::Blocked;s.sliceTicks=s.config.quantumTicks;}
void Wake(Process::Image&p){if(p.state==Process::State::Blocked)p.state=Process::State::Ready;}
void Yield(State&s){s.sliceTicks=s.config.quantumTicks;}
}
