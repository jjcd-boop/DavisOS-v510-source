#pragma once
#include "../Base/Types.hpp"
namespace Davis::CommitGuard {
enum class Phase:u8 { Idle, Armed, SwitchingCr3, Completing, Emergency };
struct State { Phase phase; u64 attempts; u64 completions; u64 faults; u64 targetCr3; u64 targetRsp0; u64 faultVector; u64 faultRip; };
extern State state;
void Reset();
bool Begin(u64 cr3,u64 rsp0);
void BeforeCr3();
void Completing();
void Complete();
bool InProgress();
void Fault(u64 vector,u64 rip);
extern "C" bool DavisCommitFaultActive();
extern "C" [[noreturn]] void DavisEmergencyCommitHalt();
}
