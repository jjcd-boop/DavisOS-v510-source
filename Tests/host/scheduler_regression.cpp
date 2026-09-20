#include <cstdio>
#include <cstdlib>
#include "../../Kernel/Scheduler.hpp"
using namespace Davis;
static void require(bool v,const char* m){ if(!v){std::fprintf(stderr,"FAIL: %s\n",m);std::exit(2);} }
static Process::Image& proc(Process::Table& t, usize i, u64 id){ auto &p=t.process[i]; p={};p.id=id;p.state=Process::State::Ready;return p; }
int main(){
 Process::Table t{}; auto&a=proc(t,0,1);auto&b=proc(t,1,2);auto&c=proc(t,2,3);
 Scheduler::State s{};Scheduler::Init(s,t,{4});
 u64 seen[4]={}; for(int i=0;i<120;i++){auto*p=Scheduler::OnTick(s);require(p,"scheduler returned null with runnable work");seen[p->id]++;}
 require(seen[1]==40&&seen[2]==40&&seen[3]==40,"round-robin fairness regression");
 require(s.ticks==120,"tick accounting regression"); require(s.switches>=29,"switch accounting regression");
 // Blocking the current task must remove it from subsequent selections.
 auto*cur=Scheduler::Current(s);require(cur,"missing current process");u64 blocked=cur->id;Scheduler::BlockCurrent(s);
 for(int i=0;i<24;i++){auto*p=Scheduler::OnTick(s);require(p,"lost runnable task after block");require(p->id!=blocked,"blocked process was scheduled");}
 Process::Image*bp=blocked==1?&a:(blocked==2?&b:&c);Scheduler::Wake(*bp);require(bp->state==Process::State::Ready,"wake failed");
 bool returned=false;for(int i=0;i<16;i++){auto*p=Scheduler::OnTick(s);if(p&&p->id==blocked)returned=true;}require(returned,"woken process never returned to CPU");
 // Yield must force a scheduling decision on the next tick.
 auto*before=Scheduler::Current(s);require(before,"missing current before yield");Scheduler::Yield(s);auto*after=Scheduler::OnTick(s);require(after,"yield produced no runnable process");require(after->id!=before->id,"yield failed to rotate with peers ready");
 // Empty table must be safe and deterministic.
 Process::Table empty{};Scheduler::State es{};Scheduler::Init(es,empty,{4});require(Scheduler::OnTick(es)==nullptr,"empty scheduler returned process");
 std::printf("PASS scheduler_regression ticks=%llu switches=%llu fairness=%llu/%llu/%llu\n",(unsigned long long)s.ticks,(unsigned long long)s.switches,(unsigned long long)seen[1],(unsigned long long)seen[2],(unsigned long long)seen[3]);
 return 0;
}
