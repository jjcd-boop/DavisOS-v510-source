#include "SmpTopology.hpp"
namespace Davis::SmpTopology {
static i32 Find(const State&s,u32 id){for(u32 i=0;i<s.cpuCount;i++)if(s.cpus[i].apicId==id)return (i32)i;return -1;}
bool Build(const AcpiMadt::State&m,u32 bsp,State&s){s={};if(!m.valid||!m.enabledCpuCount)return false;s.bspIndex=~0u;for(u32 i=0;i<m.cpuCount&&s.cpuCount<MaxCpus;i++){const auto&c=m.cpus[i];if(!c.enabled)continue;if(Find(s,c.apicId)>=0)return false;auto&d=s.cpus[s.cpuCount];d={c.apicId,c.acpiUid,c.x2Apic,CpuState::Discovered,0};if(c.apicId==bsp){d.state=CpuState::BootProcessor;s.bspIndex=s.cpuCount;s.onlineCount=1;}s.cpuCount++;}if(s.bspIndex==~0u||!s.cpuCount)return false;s.ready=true;return true;}
bool BeginStartup(State&s,u32 i){if(!s.ready||i>=s.cpuCount)return false;auto&c=s.cpus[i];if(c.state!=CpuState::Discovered&&c.state!=CpuState::Failed)return false;if(c.startupAttempts>=MaxStartupAttempts)return false;c.startupAttempts++;c.state=CpuState::StartupPending;return true;}
bool MarkOnline(State&s,u32 id){i32 i=Find(s,id);if(i<0)return false;auto&c=s.cpus[i];if(c.state==CpuState::BootProcessor||c.state==CpuState::Online)return true;if(c.state!=CpuState::StartupPending)return false;c.state=CpuState::Online;s.onlineCount++;return true;}
bool MarkFailed(State&s,u32 id){i32 i=Find(s,id);if(i<0)return false;auto&c=s.cpus[i];if(c.state!=CpuState::StartupPending)return false;c.state=CpuState::Failed;return true;}
bool AllApplicationProcessorsSettled(const State&s){if(!s.ready)return false;for(u32 i=0;i<s.cpuCount;i++){auto x=s.cpus[i].state;if(x==CpuState::Discovered||x==CpuState::StartupPending)return false;}return true;}
}
