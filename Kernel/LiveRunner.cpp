#include "LiveRunner.hpp"
#include "UserLaunch.hpp"
#include "RuntimeCurrent.hpp"
#include "LiveTrace.hpp"
namespace Davis::LiveRunner {
State state{};
extern "C" void DavisEnterPreparedUser(u64,u64,u64,u64,u64,u64);
static MultiAppRuntime::App* appA(MultiAppRuntime::State&r){for(usize i=0;i<r.appCount;i++)if(r.apps[i].kind==AppPreemptionA&&r.apps[i].ready)return &r.apps[i];return nullptr;}
bool TryStart(const BootInfo&b,MultiAppRuntime::State&r,PlatformBootstrap::State&pb){state={};LiveTrace::Reset();if(!b.liveProofRequested){state.result=Result::NotRequested;return false;}if(!pb.ready){state.result=Result::PlatformNotReady;return false;}LiveProof::Init(state.proof);if(!LiveProof::Prepare(state.proof,r)){state.result=Result::ProofNotReady;return false;}LiveActivation::Init(state.activation,100000,1);KernelPlatform::PermitLiveActivation(pb.platform,true);if(!LiveActivation::Verify(state.activation,r,pb.platform)||!LiveActivation::Arm(state.activation,r,pb.platform)){KernelPlatform::PermitLiveActivation(pb.platform,false);state.result=Result::ActivationFailed;return false;}auto*a=appA(r);if(!a||!a->loaded.process){LiveActivation::Cutoff(state.activation,pb.platform);state.result=Result::ActivationFailed;return false;}state.firstId=a->loaded.process->id;if(!TimerRuntime::Activate(pb.platform.runtime,pb.platform.activation,pb.platform.timerPlatform,*pb.platform.timer,pb.platform.preemption?*pb.platform.preemption:r.preemption,pb.platform.time)){LiveActivation::Cutoff(state.activation,pb.platform);state.result=Result::TimerFailed;return false;}state.result=Result::Entered;state.launches++;auto&f=a->initial;LiveTrace::Record(LiveTrace::Kind::EnterUser,state.firstId,a->loaded.space.pml4Physical,a->kernelRsp0,0,f.rip);DavisEnterPreparedUser(f.rip,f.rsp,f.rflags,f.cs,f.ss,a->loaded.space.pml4Physical);LiveActivation::Cutoff(state.activation,pb.platform);state.result=Result::Returned;LiveTrace::Record(LiveTrace::Kind::Returned);return true;}
}
