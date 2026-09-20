#include "SmpRuntime.hpp"
namespace Davis::SmpRuntime {
static i32 Find(const State&s,u32 id){for(u32 i=0;i<s.cpuCount;i++)if(s.cpu[i].present&&s.cpu[i].apicId==id)return (i32)i;return -1;}
bool Init(const SmpTopology::State&t,State&s){s={};if(!t.ready||!t.cpuCount||t.cpuCount>MaxCpus)return false;for(u32 i=0;i<t.cpuCount;i++){const auto&c=t.cpus[i];if(Find(s,c.apicId)>=0)return false;auto&d=s.cpu[s.cpuCount];d.apicId=c.apicId;d.logicalId=s.cpuCount;d.present=true;d.online=(c.state==SmpTopology::CpuState::BootProcessor||c.state==SmpTopology::CpuState::Online);if(d.online)s.onlineCount++;s.cpuCount++;}if(!s.onlineCount)return false;s.generation=1;s.ready=true;return Validate(s);}
bool SetOnline(State&s,u32 id,bool on){if(!s.ready)return false;i32 i=Find(s,id);if(i<0)return false;auto&c=s.cpu[i];if(c.online==on)return true;c.online=on;if(on)s.onlineCount++;else {if(!s.onlineCount)return false;s.onlineCount--;}s.generation++;return Validate(s);}
bool AccountSchedulerTick(State&s,u32 id,u64 n){i32 i=Find(s,id);if(i<0||!s.cpu[i].online||!n)return false;s.cpu[i].schedulerTicks+=n;return true;}
bool AccountInterrupt(State&s,u32 id,u64 n){i32 i=Find(s,id);if(i<0||!s.cpu[i].online||!n)return false;s.cpu[i].interrupts+=n;return true;}
const CpuLocal* Lookup(const State&s,u32 id){i32 i=Find(s,id);return i<0?nullptr:&s.cpu[i];}
bool Validate(const State&s){if(!s.ready||!s.cpuCount||s.cpuCount>MaxCpus||!s.onlineCount||s.onlineCount>s.cpuCount)return false;u32 online=0;for(u32 i=0;i<s.cpuCount;i++){if(!s.cpu[i].present||s.cpu[i].logicalId!=i)return false;for(u32 j=i+1;j<s.cpuCount;j++)if(s.cpu[i].apicId==s.cpu[j].apicId)return false;if(s.cpu[i].online)online++;}return online==s.onlineCount;}
}
