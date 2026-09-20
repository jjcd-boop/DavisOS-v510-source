#pragma once
#include "Graphics.hpp"
#include "MultiAppRuntime.hpp"
namespace Davis::LiveDiagnostics {
enum class Gate:u8 { Disabled, RuntimeMissing, RuntimeReady, PlatformBridgePending, ReadyForActivation, Active, Survived, Cutoff, Failed };
struct Snapshot { Gate gate; u64 appCount; u64 processA; u64 processB; u64 switches; u64 faults; bool separateCr3; bool separateRsp0; bool kernelMappings; };
Snapshot Inspect(const MultiAppRuntime::State&);
const char* GateText(Gate);
void DrawPanel(Graphics::Surface&,i64 x,i64 y,i64 w,const Snapshot&);
}
