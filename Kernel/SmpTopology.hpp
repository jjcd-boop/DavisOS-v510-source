#pragma once
#include "AcpiMadt.hpp"
namespace Davis::SmpTopology {
constexpr u32 MaxCpus=AcpiMadt::MaxCpus;
constexpr u32 MaxStartupAttempts=3;
enum class CpuState:u8 { Disabled, Discovered, BootProcessor, StartupPending, Online, Failed };
struct Cpu { u32 apicId; u32 acpiUid; bool x2Apic; CpuState state; u32 startupAttempts; };
struct State { Cpu cpus[MaxCpus]; u32 cpuCount; u32 onlineCount; u32 bspIndex; bool ready; };
bool Build(const AcpiMadt::State&,u32 bspApicId,State&);
bool BeginStartup(State&,u32 index);
bool MarkOnline(State&,u32 apicId);
bool MarkFailed(State&,u32 apicId);
bool AllApplicationProcessorsSettled(const State&);
}
