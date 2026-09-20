#include "LiveProof.hpp"
namespace Davis::LiveProof {
void Init(State&s){s={};s.phase=Phase::Cold;}
bool Prepare(State&s,const MultiAppRuntime::State&r){Init(s);if(!r.armed||r.result!=MultiAppRuntime::Result::Armed||!r.proof.armed){s.phase=Phase::Failed;s.failure=Failure::RuntimeNotArmed;return false;}s.aId=r.proof.a.processId;s.bId=r.proof.b.processId;if(!s.aId||!s.bId||s.aId==s.bId){s.phase=Phase::Failed;s.failure=Failure::MissingEndpoints;return false;}s.phase=Phase::Ready;return true;}
bool ObserveSwitch(State&s,u64 id,u64 cr3,u64 rsp0,PreemptionProof::State&p){if(s.phase!=Phase::Ready&&s.phase!=Phase::Running){s.phase=Phase::Failed;s.failure=Failure::WrongCurrent;return false;}if(!PreemptionProof::Observe(p,id,cr3,rsp0)){s.phase=Phase::Failed;s.failure=Failure::WrongCurrent;return false;}s.observedSwitches=p.observedSwitches;s.phase=Phase::Running;return true;}
bool ObserveFault(State&s,Process::Image&f,Process::Image&survivor,PreemptionProof::State&p){if(f.state!=Process::State::Faulted||survivor.state==Process::State::Faulted||survivor.state==Process::State::Exited){s.phase=Phase::Failed;s.failure=Failure::FaultNotContained;return false;}s.faultedId=f.id;if(!PreemptionProof::ObserveFaultIsolation(p,f.id,survivor.id)){s.phase=Phase::Failed;s.failure=Failure::FaultNotContained;return false;}s.phase=Phase::Passed;return true;}
}
