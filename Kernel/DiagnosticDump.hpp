#pragma once
#include "BootInfo.hpp"
#include "../Base/Types.hpp"
namespace Davis::DiagnosticDump {
struct State { bool enabled, runtimeReady; uptr runtimeServices; u64 sequence,persists,failures; usize length; char text[24576]; };
extern State state;
void Init(const BootInfo&);
void Event(const char* tag,const char* detail,u64 a=0,u64 b=0,u64 c=0,u64 d=0);
void Snapshot(const char* reason);
void Persist(const char* checkpoint);
}
