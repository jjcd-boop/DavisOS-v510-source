#include "BackendDiagnostics.hpp"
namespace Davis::BackendDiagnostics {
State state{};
void Init(){state={};}
void Record(Kind k,u64 pid,u64 a,u64 b,u64 c,u64 d){if(state.paused){state.dropped++;return;}state.events[state.head]={++state.next,k,pid,a,b,c,d};state.head=(state.head+1)%State::Capacity;if(state.count<State::Capacity)state.count++;}
const Event* Newest(usize back){if(back>=state.count)return nullptr;usize i=(state.head+State::Capacity-1-back)%State::Capacity;return &state.events[i];}
const char* Name(Kind k){switch(k){case Kind::Boot:return "BOOT";case Kind::AppPrepare:return "PREP";case Kind::AppClick:return "CLICK";case Kind::LaunchState:return "LAUNCH";case Kind::Dispatch:return "DISPATCH";case Kind::EnterUser:return "ENTER3";case Kind::ResumeUser:return "RESUME3";case Kind::Syscall:return "SYSCALL";case Kind::WindowCreate:return "WINCREATE";case Kind::WindowPresent:return "PRESENT";case Kind::ProcessExit:return "EXIT";case Kind::UserFault:return "FAULT";case Kind::Scheduler:return "SCHED";case Kind::Shutdown:return "SHUTDOWN";case Kind::Error:return "ERROR";default:return "?";}}
}
