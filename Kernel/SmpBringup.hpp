#pragma once
#include "ApBootstrap.hpp"
#include "ApLowMemory.hpp"
#include "ApRendezvous.hpp"
#include "LapicIpi.hpp"
namespace Davis::SmpBringup {
constexpr u32 DefaultRendezvousPollLimit=100000;
struct Platform { bool(*installLowMemory)(void*,u64,const u8*,u32); bool(*poll)(void*,u32,u64,u64,u32); void* context; u32 pollLimit; };
struct Result { u32 attempted; u32 online; u32 failed; u32 ipis; };
bool BringUpApplicationProcessors(SmpTopology::State&,SmpRuntime::State&,ApBootstrap::State&,ApRendezvous::Mailbox&,u64 trampolinePhysical,u64 cr3,const LapicIpi::Transport&,const Platform&,Result&);
}
