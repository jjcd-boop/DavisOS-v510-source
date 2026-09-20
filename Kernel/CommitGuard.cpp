#include "CommitGuard.hpp"
namespace Davis::CommitGuard {
State state{};
void Reset(){state={};state.phase=Phase::Idle;}
bool Begin(u64 cr3,u64 rsp0){if(InProgress())return false;state.phase=Phase::Armed;state.attempts++;state.targetCr3=cr3;state.targetRsp0=rsp0;state.faultVector=0;state.faultRip=0;return true;}
void BeforeCr3(){if(state.phase==Phase::Armed)state.phase=Phase::SwitchingCr3;}
void Completing(){if(state.phase==Phase::Armed||state.phase==Phase::SwitchingCr3)state.phase=Phase::Completing;}
void Complete(){if(state.phase!=Phase::Emergency){state.phase=Phase::Idle;state.completions++;state.targetCr3=0;state.targetRsp0=0;}}
bool InProgress(){return state.phase==Phase::Armed||state.phase==Phase::SwitchingCr3||state.phase==Phase::Completing;}
void Fault(u64 vector,u64 rip){state.faults++;state.faultVector=vector;state.faultRip=rip;state.phase=Phase::Emergency;}
extern "C" bool DavisCommitFaultActive(){return state.phase==Phase::Emergency;}
extern "C" [[noreturn]] void DavisEmergencyCommitHalt(){
#if defined(__x86_64__) && !defined(DAVIS_HOST_TEST)
 __asm__ __volatile__("cli":::"memory"); for(;;)__asm__ __volatile__("hlt");
#else
 for(;;){}
#endif
}
}
