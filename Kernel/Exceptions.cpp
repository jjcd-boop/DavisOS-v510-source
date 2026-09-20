#include "Exceptions.hpp"
#include "TrapExit.hpp"
#include "LiveTrace.hpp"
#include "CommitGuard.hpp"
#include "StackRuntime.hpp"
namespace Davis::Exceptions {
State state{};
void SetCurrent(Process::Image*p){state.current=p;state.userFault=false;}
bool IsUserFrame(const Frame&f){return (f.cs&3u)==3u;}
Class Classify(const Frame&f){if(f.vector==2)return Class::NonMaskable;if(f.vector==18)return Class::MachineCheck;if(IsUserFrame(f))return Class::UserRecoverable;return Class::KernelFatal;}
void InstallCoreGates(Idt::Table&t,u16 kc){
 Idt::SetInterruptGate(t,0,DavisExceptionDE,kc,0); Idt::SetInterruptGate(t,2,DavisExceptionNMI,kc,0,2); Idt::SetInterruptGate(t,3,DavisExceptionBP,kc,3);
 Idt::SetInterruptGate(t,4,DavisExceptionOF,kc,3); Idt::SetInterruptGate(t,5,DavisExceptionBR,kc,0); Idt::SetInterruptGate(t,6,DavisExceptionUD,kc,0);
 Idt::SetInterruptGate(t,7,DavisExceptionNM,kc,0); Idt::SetInterruptGate(t,11,DavisExceptionNP,kc,0); Idt::SetInterruptGate(t,12,DavisExceptionSS,kc,0);
 Idt::SetInterruptGate(t,13,DavisExceptionGP,kc,0); Idt::SetInterruptGate(t,14,DavisExceptionPF,kc,0); Idt::SetInterruptGate(t,17,DavisExceptionAC,kc,0);
 Idt::SetInterruptGate(t,18,DavisExceptionMC,kc,0,3); Idt::SetInterruptGate(t,19,DavisExceptionXM,kc,0);
}
extern "C" void DavisExceptionDispatch(Frame*f){if(!f)return;if(f->vector!=2&&f->vector!=8&&f->vector!=18&&state.current&&!StackRuntime::Validate(state.current->id,(u64)(uptr)f,StackRuntime::EntryKind::Exception))CommitGuard::DavisEmergencyCommitHalt();state.count++;state.lastVector=f->vector;state.lastError=f->error;state.lastRip=f->rip;
#if defined(__x86_64__) && !defined(DAVIS_HOST_TEST)
 if(f->vector==14)asm volatile("mov %%cr2,%0":"=r"(state.lastCr2));
#endif
 auto c=Classify(*f);
 if(c==Class::NonMaskable){state.nmis++;return;}
 if(c==Class::MachineCheck){state.machineChecks++;state.fatalKernelFault=true;return;}
 if(c==Class::UserRecoverable){state.userFault=true;state.userFaults++;if(state.current)state.current->lastFaultVector=f->vector;LiveTrace::Record(LiveTrace::Kind::UserFault,state.current?state.current->id:0,0,0,f->vector,f->rip);if(state.current)TrapExit::RequestFault(f->vector);return;}
 state.kernelFaults++;if(CommitGuard::InProgress())CommitGuard::Fault(f->vector,f->rip);state.fatalKernelFault=true;
}
}
