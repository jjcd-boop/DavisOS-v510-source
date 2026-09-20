#include <cstdio>
#include <thread>
#include <vector>
#include "Kernel/SmpScheduler.hpp"
using namespace Davis;
int main(){SmpTopology::State top{};top.ready=true;top.cpuCount=4;top.onlineCount=4;top.bspIndex=0;for(u32 i=0;i<4;i++){top.cpus[i].apicId=10+i;top.cpus[i].state=i?SmpTopology::CpuState::Online:SmpTopology::CpuState::BootProcessor;}
 SmpRuntime::State rt{};if(!SmpRuntime::Init(top,rt))return 1;Process::Table pt{};for(u32 i=0;i<8;i++){pt.process[i].id=i+1;pt.process[i].state=Process::State::Ready;}Scheduler::State sched{};Scheduler::Init(sched,pt,{4});SmpScheduler::State smp{};if(!SmpScheduler::Init(smp,sched,rt))return 2;
 constexpr int loops=25000;std::vector<std::thread> ts;for(u32 cpu=0;cpu<4;cpu++)ts.emplace_back([&,cpu](){for(int i=0;i<loops;i++){SmpScheduler::Tick(smp,10+cpu);if((i%997)==0)SmpScheduler::Yield(smp,10+cpu);}});for(auto&t:ts)t.join();
 if(smp.dispatches!=100000||sched.ticks!=100000)return 3;for(u32 i=0;i<4;i++)if(!rt.cpu[i].schedulerTicks)return 4;if(!SmpScheduler::Validate(smp))return 5;if(!SmpRuntime::SetOnline(rt,13,false))return 6;auto before=smp.rejectedTicks;if(SmpScheduler::Tick(smp,13)!=nullptr||smp.rejectedTicks!=before+1)return 7;
 std::printf("PASS smp_scheduler ticks=%llu dispatches=%llu contentions=%llu\n",(unsigned long long)sched.ticks,(unsigned long long)smp.dispatches,(unsigned long long)smp.gate.contentions);return 0;}
