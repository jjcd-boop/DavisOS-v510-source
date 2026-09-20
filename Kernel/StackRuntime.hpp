#pragma once
#include "../Base/Types.hpp"
#include "StackGuard.hpp"
namespace Davis::StackRuntime {
enum class EntryKind:u8 { Syscall, Timer, Exception, ContextCommit };
struct Record { u64 processId, low, high; volatile u64* canary; bool valid; };
struct State { Record record[64]; u64 checks, failures, canaryFailures, rangeFailures; EntryKind lastKind; u64 lastProcessId,lastRsp; bool fatal; };
extern State state;
void Init();
bool Register(u64 processId,const StackGuard::Stack&);
bool Validate(u64 processId,u64 rsp,EntryKind);
bool ValidateTarget(u64 processId,u64 rsp0,EntryKind);
bool ValidateAnyTarget(u64 rsp0,EntryKind);
}
