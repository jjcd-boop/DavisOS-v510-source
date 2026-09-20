#pragma once
#include "../Base/Types.hpp"
#include "SmpTopology.hpp"
#include "SmpRuntime.hpp"
namespace Davis::ApBootstrap {
constexpr u32 MaxCpus=SmpTopology::MaxCpus;
constexpr u64 MinStackBytes=4096;
constexpr u64 DefaultStackBytes=16384;
constexpr u64 MaxStackBytes=65536;
enum class Stage:u8 { Empty, Prepared, StartupSent, EnteredLongMode, Rendezvous, Online, Failed };
struct CpuBoot { u32 apicId; u32 logicalId; u64 stackBase; u64 stackTop; u64 entryPoint; u64 token; Stage stage; u32 attempts; };
struct State { CpuBoot cpu[MaxCpus]; u32 count; u32 online; u64 generation; bool ready; };
bool Init(const SmpTopology::State&,u64 stackRegionBase,u64 stackBytes,u64 entryPoint,State&);
bool StartupSent(State&,SmpTopology::State&,u32 apicId);
bool EnteredLongMode(State&,u32 apicId,u64 token);
bool Rendezvous(State&,u32 apicId,u64 token);
bool MarkOnline(State&,SmpTopology::State&,SmpRuntime::State&,u32 apicId,u64 token);
bool MarkFailed(State&,SmpTopology::State&,u32 apicId);
const CpuBoot* Lookup(const State&,u32 apicId);
bool Validate(const State&);
}
