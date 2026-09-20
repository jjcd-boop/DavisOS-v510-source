#include "UserTrap.hpp"
#include "Syscall.hpp"
#include "TrapExit.hpp"
#include "StackRuntime.hpp"
#include "CommitGuard.hpp"
#include "Preemption.hpp"
#include "CpuContext.hpp"
namespace Davis::UserTrap {
State state{};void SetCurrent(Process::Image*p){state.current=p;state.exitRequested=false;}
extern "C" void DavisInt80Dispatch(Frame*f){if(!f)return;if(state.current&&!StackRuntime::Validate(state.current->id,(u64)(uptr)f,StackRuntime::EntryKind::Syscall))CommitGuard::DavisEmergencyCommitHalt();Syscall::Frame q{f->rax,f->rdi,f->rsi,f->rdx,f->r10,f->r8,f->r9};auto r=Syscall::DispatchForProcess(q,state.current);if(r.error==Syscall::ErrorPermissionDenied)state.capabilityDenials++;state.syscallCount++;state.lastNumber=q.number;state.lastValue=r.value;state.lastError=r.error;f->rax=(u64)r.value;f->rdx=r.error;if(q.number==Syscall::Exit&&state.current){TrapExit::RequestExit((int)q.arg0);state.exitRequested=true;}else if(q.number==Syscall::Yield&&state.current){
 CpuContext::Frame saved{}; const u64* hw=(const u64*)(f+1);
 saved.r15=f->r15;saved.r14=f->r14;saved.r13=f->r13;saved.r12=f->r12;saved.r11=f->r11;saved.r10=f->r10;saved.r9=f->r9;saved.r8=f->r8;
 saved.rbp=f->rbp;saved.rdi=f->rdi;saved.rsi=f->rsi;saved.rdx=f->rdx;saved.rcx=f->rcx;saved.rbx=f->rbx;saved.rax=f->rax;
 saved.vector=0x80;saved.error=0;saved.rip=hw[0];saved.cs=hw[1];saved.rflags=hw[2];saved.rsp=hw[3];saved.ss=hw[4];
 if(!Preemption::SaveGlobal(state.current->id,saved)){TrapExit::RequestFault(0x100);return;}
 TrapExit::RequestYield();
}}
}
