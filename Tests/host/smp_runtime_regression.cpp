#include <cstdio>
#include "Kernel/SmpRuntime.hpp"
using namespace Davis;
int main(){
 AcpiMadt::State m{};m.valid=true;m.enabledCpuCount=4;m.cpuCount=4;for(u32 i=0;i<4;i++){m.cpus[i].enabled=true;m.cpus[i].apicId=i;m.cpus[i].acpiUid=i;}
 SmpTopology::State t{};if(!SmpTopology::Build(m,0,t))return 1;
 for(u32 i=1;i<4;i++){if(!SmpTopology::BeginStartup(t,i)||!SmpTopology::MarkOnline(t,i))return 2;}
 for(int cycle=0;cycle<10000;cycle++){
  SmpRuntime::State s{};if(!SmpRuntime::Init(t,s)||s.onlineCount!=4)return 3;
  for(u32 cpu=0;cpu<4;cpu++){if(!SmpRuntime::AccountSchedulerTick(s,cpu,100)||!SmpRuntime::AccountInterrupt(s,cpu,7))return 4;}
  if(!SmpRuntime::SetOnline(s,3,false)||s.onlineCount!=3)return 5;
  if(SmpRuntime::AccountSchedulerTick(s,3,1))return 6;
  if(!SmpRuntime::SetOnline(s,3,true)||s.onlineCount!=4||!SmpRuntime::Validate(s))return 7;
  for(u32 cpu=0;cpu<4;cpu++){auto*c=SmpRuntime::Lookup(s,cpu);if(!c||!c->online||c->schedulerTicks<100||c->interrupts!=7)return 8;}
 }
 std::puts("PASS smp_runtime per-cpu-accounting hotplug-simulation cycles=10000");return 0;
}
