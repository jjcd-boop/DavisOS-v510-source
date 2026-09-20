#include "Kernel/ApBootstrap.hpp"
#include <cstdio>
using namespace Davis;
static void makeTopology(SmpTopology::State&t){t={};t.ready=true;t.cpuCount=4;t.onlineCount=1;t.bspIndex=0;for(u32 i=0;i<4;i++)t.cpus[i]={i,i,false,i?SmpTopology::CpuState::Discovered:SmpTopology::CpuState::BootProcessor,0};}
int main(){
 for(u32 cycle=0;cycle<10000;cycle++){
  SmpTopology::State t{};makeTopology(t);SmpRuntime::State r{};if(!SmpRuntime::Init(t,r))return 1;ApBootstrap::State b{};
  if(!ApBootstrap::Init(t,0x1000000,16384,0xffffffff80001000ull,b)||b.online!=1)return 2;
  for(u32 id=1;id<4;id++){auto*c=ApBootstrap::Lookup(b,id);if(!c||!ApBootstrap::StartupSent(b,t,id))return 3;c=ApBootstrap::Lookup(b,id);u64 token=c->token;if(ApBootstrap::EnteredLongMode(b,id,token^1))return 4;if(!ApBootstrap::EnteredLongMode(b,id,token)||!ApBootstrap::Rendezvous(b,id,token)||!ApBootstrap::MarkOnline(b,t,r,id,token))return 5;}
  if(b.online!=4||r.onlineCount!=4||t.onlineCount!=4||!ApBootstrap::Validate(b))return 6;
 }
 SmpTopology::State t{};makeTopology(t);SmpRuntime::State r{};SmpRuntime::Init(t,r);ApBootstrap::State b{};
 if(ApBootstrap::Init(t,0x1000001,16384,0x1000,b)||ApBootstrap::Init(t,0x1000000,8192+1,0x1000,b)||ApBootstrap::Init(t,0x1000000,131072,0x1000,b))return 7;
 if(!ApBootstrap::Init(t,0x2000000,4096,0x2000,b)||!ApBootstrap::StartupSent(b,t,1))return 8;auto*c=ApBootstrap::Lookup(b,1);u64 tok=c->token;if(!ApBootstrap::MarkFailed(b,t,1))return 9;
 if(!ApBootstrap::StartupSent(b,t,1)||!ApBootstrap::EnteredLongMode(b,1,tok)||!ApBootstrap::Rendezvous(b,1,tok)||!ApBootstrap::MarkOnline(b,t,r,1,tok))return 10;
 std::printf("PASS ap_bootstrap cycles=10000 cpus=4 rendezvous=30000 stack=16384 retry_recovery=1\n");return 0;
}
