#pragma once
#include "../Base/Types.hpp"
namespace Davis::BackendDiagnostics {
enum class Kind:u8 { Boot=1,AppPrepare,AppClick,LaunchState,Dispatch,EnterUser,ResumeUser,Syscall,WindowCreate,WindowPresent,ProcessExit,UserFault,Scheduler,Shutdown,Error };
struct Event { u64 seq; Kind kind; u64 pid; u64 a; u64 b; u64 c; u64 d; };
struct State { static constexpr usize Capacity=256; Event events[Capacity]; u64 next; usize head,count; bool paused; u64 dropped; };
extern State state;
void Init();
void Record(Kind,u64 pid=0,u64 a=0,u64 b=0,u64 c=0,u64 d=0);
const Event* Newest(usize back);
const char* Name(Kind);
}
