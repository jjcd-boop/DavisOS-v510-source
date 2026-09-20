#pragma once
#include "../Base/Types.hpp"
namespace Davis::LiveTrace {
enum class Kind:u8 { None, EnterUser, SwitchCommit, UserFault, RecoveryCommit, Cutoff, Returned };
struct Event { Kind kind; u64 sequence; u64 processId; u64 cr3; u64 rsp0; u64 vector; u64 rip; };
struct State { static constexpr usize Capacity=32; Event events[Capacity]; u64 nextSequence; usize count; usize head; u64 switchCommits; u64 userFaults; u64 recoveries; u64 lastProcessId; u64 lastCr3; u64 lastRsp0; u64 lastVector; bool kernelReturned; };
extern State state;
void Reset();
void Record(Kind,u64 processId=0,u64 cr3=0,u64 rsp0=0,u64 vector=0,u64 rip=0);
const Event* Latest();
}
