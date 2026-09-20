#pragma once
#include "BootInfo.hpp"
#include "MultiAppRuntime.hpp"
#include "PlatformBootstrap.hpp"
#include "LiveActivation.hpp"
#include "LiveProof.hpp"
namespace Davis::LiveRunner {
enum class Result:u8 { NotRequested, PlatformNotReady, ProofNotReady, ActivationFailed, TimerFailed, Entered, Returned };
struct State { Result result; LiveActivation::State activation; LiveProof::State proof; u64 firstId; u64 launches; };
extern State state;
bool TryStart(const BootInfo&,MultiAppRuntime::State&,PlatformBootstrap::State&);
}
