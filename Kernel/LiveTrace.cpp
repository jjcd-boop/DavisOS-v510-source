#include "LiveTrace.hpp"
namespace Davis::LiveTrace {
State state{};
void Reset(){state={};}
void Record(Kind k,u64 pid,u64 cr3,u64 rsp0,u64 vec,u64 rip){Event e{k,++state.nextSequence,pid,cr3,rsp0,vec,rip};state.events[state.head]=e;state.head=(state.head+1)%State::Capacity;if(state.count<State::Capacity)state.count++;state.lastProcessId=pid?pid:state.lastProcessId;state.lastCr3=cr3?cr3:state.lastCr3;state.lastRsp0=rsp0?rsp0:state.lastRsp0;state.lastVector=vec?vec:state.lastVector;if(k==Kind::SwitchCommit)state.switchCommits++;if(k==Kind::UserFault)state.userFaults++;if(k==Kind::RecoveryCommit)state.recoveries++;if(k==Kind::Returned)state.kernelReturned=true;}
const Event* Latest(){if(!state.count)return nullptr;usize i=(state.head+State::Capacity-1)%State::Capacity;return &state.events[i];}
}
