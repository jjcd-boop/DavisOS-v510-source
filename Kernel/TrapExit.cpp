#include "TrapExit.hpp"
#include "IsolationGuard.hpp"
#include "DriverManager.hpp"
#include "Syscall.hpp"
namespace Davis::TrapExit {
State state{};
void Arm(Process::Image*p){state.current=p;state.requested=false;state.reason=Reason::None;state.code=0;state.vector=0;}
void RequestExit(int code){state.requested=true;state.reason=Reason::ProcessExit;state.code=code;if(state.current){Syscall::RevokeProcessObjects(state.current->id);Process::MarkExited(*state.current,code);}}
void RequestYield(){state.requested=true;state.reason=Reason::CooperativeYield;state.code=0;state.vector=0;}
void RequestSystemSlice(){state.requested=true;state.reason=Reason::SystemSlice;state.code=0;state.vector=0;}
void RequestFault(u64 v){state.requested=true;state.reason=Reason::UserFault;state.vector=v;state.code=-(int)v;if(state.current){IsolationGuard::RecordFault(*state.current,IsolationGuard::FaultKind::UserFault,v);if(state.current->domain==SecurityDomain::Kind::Driver)DriverManager::MarkFailed(state.current->id);Syscall::RevokeProcessObjects(state.current->id);state.current->state=Process::State::Faulted;state.current->exitCode=state.code;}}
bool ShouldAbandonUser(){return state.requested;}
void Clear(){auto*p=state.current;state={};state.current=p;}
}
extern "C" Davis::u64 DavisTrapShouldAbandon(){return Davis::TrapExit::ShouldAbandonUser()?1:0;}
