#pragma once
#include "SmpTopology.hpp"
namespace Davis::SmpRuntime {
constexpr u32 MaxCpus=SmpTopology::MaxCpus;
struct CpuLocal { u32 apicId; u32 logicalId; bool present; bool online; u64 schedulerTicks; u64 interrupts; };
struct State { CpuLocal cpu[MaxCpus]; u32 cpuCount; u32 onlineCount; u64 generation; bool ready; };
bool Init(const SmpTopology::State&,State&);
bool SetOnline(State&,u32 apicId,bool online);
bool AccountSchedulerTick(State&,u32 apicId,u64 ticks=1);
bool AccountInterrupt(State&,u32 apicId,u64 count=1);
const CpuLocal* Lookup(const State&,u32 apicId);
bool Validate(const State&);
}
