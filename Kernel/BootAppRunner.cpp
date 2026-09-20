#include "BootAppRunner.hpp"
#include "RuntimeCurrent.hpp"
#include "UserLaunch.hpp"
#include "Syscall.hpp"
#include "Preemption.hpp"
#include "Scheduler.hpp"
#include "TrapExit.hpp"
#include "DesktopService.hpp"
#include "StorageService.hpp"
#include "AudioService.hpp"
#include "NetworkService.hpp"
#include "SystemTasks.hpp"
#include "../Arch/x86_64/Cpu.hpp"
#include "BackendDiagnostics.hpp"
#include "DxeServices.hpp"
namespace Davis::BootAppRunner {
State state{};
extern "C" void DavisEnterPreparedUser(u64,u64,u64,u64,u64,u64);
extern "C" void DavisResumePreparedUser(CpuContext::Frame*,u64);
extern "C" u64 DavisPendingKernelRsp0;
static MultiAppRuntime::App* appFor(MultiAppRuntime::State&r,u64 id){for(usize i=0;i<r.appCount;i++)if(r.apps[i].loaded.process&&r.apps[i].loaded.process->id==id)return &r.apps[i];return nullptr;}
static void quarantineNonGeneric(MultiAppRuntime::State&r,bool block){for(usize i=0;i<r.appCount;i++){auto&a=r.apps[i];auto*p=a.loaded.process;if(!p||a.kind!=AppPreemptionA&&a.kind!=AppPreemptionB)continue;if(block&&p->state==Process::State::Ready)p->state=Process::State::Blocked;else if(!block&&p->state==Process::State::Blocked)p->state=Process::State::Ready;}}
static Process::Image* chooseSurvivor(MultiAppRuntime::State&r,AppCohort::State&c){
 AppCohort::RetireTerminal(c,r);if(!c.runnable)return nullptr;
 auto*p=Scheduler::PickNext(r.scheduler);if(!p)return AppCohort::FirstRunnable(c,r);
 for(usize n=0;n<c.count;n++)if(c.memberIds[n]==p->id)return p;
 return AppCohort::FirstRunnable(c,r);
}
static void adopt(MultiAppRuntime::State&r,PlatformBootstrap::State&pb,MultiAppRuntime::App&a,Process::Image&p){
 if(auto*old=Scheduler::Current(r.scheduler);old&&old->id!=p.id&&old->state==Process::State::Running)old->state=Process::State::Ready;
 p.state=Process::State::Running;r.scheduler.currentId=p.id;r.scheduler.sliceTicks=0;pb.gdt.tss.rsp0=a.kernelRsp0;
 Preemption::AdoptActive(r.preemption,a.loaded.space.pml4Physical,a.kernelRsp0);RuntimeCurrent::Set(&p);
}
static bool handshakeComplete(u64 pid){return DxeServices::RaiseFirstWindowForOwner(pid)&&DxeServices::OwnerHasPresented(pid);}
void RunGeneric(MultiAppRuntime::State&r,PlatformBootstrap::State&pb){
 state={};if(!pb.ready){state.result=Result::PlatformNotReady;return;}AppCohort::Build(state.cohort,r);if(!state.cohort.count){state.result=Result::NoGenericApps;return;}
 auto*first=AppCohort::FirstRunnable(state.cohort,r);auto*a=first?appFor(r,first->id):nullptr;if(!first||!a){state.result=Result::NoGenericApps;return;}
 state.firstProcessId=first->id;BackendDiagnostics::Record(BackendDiagnostics::Kind::Dispatch,first->id,a?a->kind:0,1);const u64 kernelCr3=UserLaunch::ReadCr3();quarantineNonGeneric(r,true);adopt(r,pb,*a,*first);
 KernelPlatform::PermitLiveActivation(pb.platform,true);pb.platform.contextSwitch.hardwareEnabled=true;r.preemption.enabled=true;
 if(!TimerRuntime::Activate(pb.platform.runtime,pb.platform.activation,pb.platform.timerPlatform,*pb.platform.timer,r.preemption,pb.platform.time)){pb.platform.contextSwitch.hardwareEnabled=false;r.preemption.enabled=false;KernelPlatform::PermitLiveActivation(pb.platform,false);quarantineNonGeneric(r,false);state.result=Result::TimerActivationFailed;return;}
 state.concurrent=state.cohort.count>1;state.dispatches++;
 auto*firstSlot=Preemption::FindSlot(r.preemption,first->id);
 BackendDiagnostics::Record(BackendDiagnostics::Kind::EnterUser,first->id,a->initial.rip,a->initial.rsp);
 if(firstSlot&&firstSlot->valid&&firstSlot->frame.rip)DavisResumePreparedUser(&firstSlot->frame,firstSlot->cr3);
 else{DavisPendingKernelRsp0=a->kernelRsp0;DavisEnterPreparedUser(a->initial.rip,a->initial.rsp,a->initial.rflags,a->initial.cs,a->initial.ss,a->loaded.space.pml4Physical);}
 for(;;){
  if(TrapExit::state.reason==TrapExit::Reason::CooperativeYield){DesktopService::Tick();}
  else if(TrapExit::state.reason==TrapExit::Reason::SystemSlice){SystemTasks::Kind taskKind; if(SystemTasks::PickNext(taskKind)){switch(taskKind){case SystemTasks::Kind::Desktop: DesktopService::Tick(); break; case SystemTasks::Kind::Storage: StorageService::PumpOnce(); break; case SystemTasks::Kind::Audio: AudioService::PumpOnce(); break; case SystemTasks::Kind::Network: NetworkService::PumpOnce(); break; default: break;} SystemTasks::NoteRun(taskKind);}}
  auto*returned=RuntimeCurrent::Get();state.returnedProcessId=returned?returned->id:0;if(returned&&(returned->state==Process::State::Exited||returned->state==Process::State::Faulted))state.isolatedTerminations++;
  AppCohort::RetireTerminal(state.cohort,r);if(!state.cohort.runnable)break;
  auto*next=chooseSurvivor(r,state.cohort);auto*na=next?appFor(r,next->id):nullptr;auto*slot=next?Preemption::FindSlot(r.preemption,next->id):nullptr;
  if(!next||!na||!slot||!slot->valid||!slot->cr3||!slot->kernelRsp0){AppCohort::AbortSurvivors(state.cohort,r,-126);break;}
  adopt(r,pb,*na,*next);state.dispatches++;BackendDiagnostics::Record(BackendDiagnostics::Kind::ResumeUser,next->id,slot->frame.rip,slot->frame.rsp);DavisResumePreparedUser(&slot->frame,slot->cr3);
 }
 Cpu::DisableInterrupts();LocalApicTimer::Stop(*pb.platform.timer);pb.platform.timerPlatform.interruptsEnabled=false;pb.platform.contextSwitch.hardwareEnabled=false;r.preemption.enabled=false;KernelPlatform::PermitLiveActivation(pb.platform,false);UserLaunch::WriteCr3(kernelCr3);
 quarantineNonGeneric(r,false);state.switches=r.preemption.contextSwitches;state.timerInterrupts=r.preemption.timerInterrupts;r.scheduler.currentId=0;r.scheduler.sliceTicks=0;RuntimeCurrent::Set(nullptr);state.cohort.armed=false;state.result=Result::CohortReturned;
}
bool LaunchKind(MultiAppRuntime::State&r,PlatformBootstrap::State&pb,u32 kind){
 if(!pb.ready)return false; MultiAppRuntime::App*a=nullptr;for(usize i=0;i<r.appCount;i++)if(r.apps[i].kind==kind){a=&r.apps[i];break;}
 if(!a||!a->ready||!a->loaded.process)return false;auto&p=*a->loaded.process;if(p.state!=Process::State::Ready&&p.state!=Process::State::Created&&p.state!=Process::State::Blocked)return false;
 const u64 kernelCr3=UserLaunch::ReadCr3();BackendDiagnostics::Record(BackendDiagnostics::Kind::Dispatch,p.id,kind,(u64)p.state);adopt(r,pb,*a,p);
 KernelPlatform::PermitLiveActivation(pb.platform,true);pb.platform.contextSwitch.hardwareEnabled=true;r.preemption.enabled=true;
 if(!TimerRuntime::Activate(pb.platform.runtime,pb.platform.activation,pb.platform.timerPlatform,*pb.platform.timer,r.preemption,pb.platform.time)){pb.platform.contextSwitch.hardwareEnabled=false;r.preemption.enabled=false;KernelPlatform::PermitLiveActivation(pb.platform,false);p.state=Process::State::Ready;return false;}
 constexpr u32 MaxHandshakeSlices=32; bool ok=true;
 for(u32 slice=0;slice<MaxHandshakeSlices;slice++){
  auto*slot=Preemption::FindSlot(r.preemption,p.id);
  if(!slot||!slot->valid||!slot->frame.rip||!slot->cr3||!slot->kernelRsp0){ok=false;BackendDiagnostics::Record(BackendDiagnostics::Kind::Error,p.id,kind,0x53544B31);break;}
  adopt(r,pb,*a,p);BackendDiagnostics::Record(slice?BackendDiagnostics::Kind::ResumeUser:BackendDiagnostics::Kind::EnterUser,p.id,slot->frame.rip,slot->frame.rsp,slice);
  DavisResumePreparedUser(&slot->frame,slot->cr3);
  const auto reason=TrapExit::state.reason;
  TrapExit::Clear();
  if(p.state==Process::State::Faulted||p.state==Process::State::Exited){ok=false;break;}
  if(reason==TrapExit::Reason::CooperativeYield||reason==TrapExit::Reason::SystemSlice){
   // Pump the compositor only. Full DesktopRuntime::PumpOnce re-enters Update
   // on the same click and can nest LaunchKind / redraw the whole framebuffer
   // with interrupts live.
   DesktopService::Tick();
   if(handshakeComplete(p.id)){BackendDiagnostics::Record(BackendDiagnostics::Kind::WindowPresent,p.id,kind,slice+1);break;}
   continue;
  }
  if(handshakeComplete(p.id)){BackendDiagnostics::Record(BackendDiagnostics::Kind::WindowPresent,p.id,kind,slice+1);break;}
  if(reason!=TrapExit::Reason::None){ok=false;break;}
 }
 TimerRuntime::ResetAfterSlice(pb.platform.runtime,pb.platform.activation,pb.platform.timerPlatform,*pb.platform.timer);pb.platform.contextSwitch.hardwareEnabled=false;r.preemption.enabled=false;KernelPlatform::PermitLiveActivation(pb.platform,false);UserLaunch::WriteCr3(kernelCr3);RuntimeCurrent::Set(nullptr);r.scheduler.currentId=0;r.scheduler.sliceTicks=0;
 if(p.state==Process::State::Running)p.state=Process::State::Ready;BackendDiagnostics::Record(BackendDiagnostics::Kind::Scheduler,p.id,(u64)p.state,(u64)TrapExit::state.reason);return ok&&p.state!=Process::State::Faulted&&p.state!=Process::State::Exited;
}

}
