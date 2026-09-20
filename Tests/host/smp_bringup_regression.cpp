#include "Kernel/SmpBringup.hpp"
#include <cstdio>
#include <cstring>
using namespace Davis;
struct Ctx { ApRendezvous::Mailbox* m; ApBootstrap::State* b; u8 low[4096]; u32 writes; u32 polls; bool failOne; };
static bool install(void*v,u64,const u8*p,u32 n){auto*c=(Ctx*)v;if(n!=4096)return false;std::memcpy(c->low,p,n);c->writes++;return true;}
static bool poll(void*v,u32 apic,u64 token,u64 gen,u32){auto*c=(Ctx*)v;c->polls++;auto*boot=ApBootstrap::Lookup(*c->b,apic);if(!boot)return false;if(c->failOne&&apic==3)return false;return ApRendezvous::Publish(*c->m,boot->logicalId,apic,token,gen,ApRendezvous::Stage::Rendezvous);}
static bool icr(void*,u64){return true;} static void delay(void*,u32){}
int main(){u64 cycles=0,online=0,failed=0;for(u32 z=0;z<10000;z++){SmpTopology::State t{};t.ready=true;t.cpuCount=4;t.onlineCount=1;t.bspIndex=0;for(u32 i=0;i<4;i++){t.cpus[i].apicId=i;t.cpus[i].acpiUid=i;t.cpus[i].state=i?SmpTopology::CpuState::Discovered:SmpTopology::CpuState::BootProcessor;}SmpRuntime::State r{};if(!SmpRuntime::Init(t,r))return 1;ApBootstrap::State b{};if(!ApBootstrap::Init(t,0x200000,16384,0xffffffff80001000ull,b))return 2;ApRendezvous::Mailbox m{};if(!ApRendezvous::Init(m,4))return 3;Ctx c{&m,&b,{},0,0,(z%97)==0};LapicIpi::Transport l{LapicIpi::Mode::XApic,0xfee00000,icr,delay,&c};SmpBringup::Platform p{install,poll,&c,1000};SmpBringup::Result x{};if(!SmpBringup::BringUpApplicationProcessors(t,r,b,m,0x8000,0x1000,l,p,x))return 4;if(x.attempted!=3||x.online+x.failed!=3)return 5;if(c.failOne){if(x.failed!=1||x.online!=2)return 6;}else if(x.failed||x.online!=3)return 7;cycles++;online+=x.online;failed+=x.failed;}
ApRendezvous::Mailbox m{};if(!ApRendezvous::Init(m,2))return 8;if(!ApRendezvous::Publish(m,1,7,11,1,ApRendezvous::Stage::Rendezvous))return 9;if(ApRendezvous::Publish(m,1,8,11,1,ApRendezvous::Stage::Online))return 10;if(!ApRendezvous::Observe(m,1,7,11,1,ApRendezvous::Stage::Rendezvous))return 11;
std::printf("PASS smp_bringup cycles=%llu online=%llu injected_failures=%llu\n",(unsigned long long)cycles,(unsigned long long)online,(unsigned long long)failed);}
