#pragma once
#include "../Base/Types.hpp"
#include "Process.hpp"
#include "Idt.hpp"
namespace Davis::Exceptions {
struct Frame { u64 r15,r14,r13,r12,r11,r10,r9,r8,rbp,rdi,rsi,rdx,rcx,rbx,rax; u64 vector,error,rip,cs,rflags,rsp,ss; };
enum class Class:u8 { UserRecoverable, KernelFatal, NonMaskable, MachineCheck };
struct State { Process::Image* current; u64 count,lastVector,lastError,lastRip,lastCr2; u64 userFaults,kernelFaults,nmis,machineChecks; bool userFault; bool fatalKernelFault; };
extern State state;
void SetCurrent(Process::Image*); bool IsUserFrame(const Frame&); Class Classify(const Frame&);
void InstallCoreGates(Idt::Table&,u16 kernelCode);
extern "C" void DavisExceptionDispatch(Frame*);
extern "C" void DavisExceptionDE(); extern "C" void DavisExceptionNMI(); extern "C" void DavisExceptionBP(); extern "C" void DavisExceptionOF();
extern "C" void DavisExceptionBR(); extern "C" void DavisExceptionUD(); extern "C" void DavisExceptionNM(); extern "C" void DavisExceptionNP();
extern "C" void DavisExceptionSS(); extern "C" void DavisExceptionGP(); extern "C" void DavisExceptionPF(); extern "C" void DavisExceptionAC();
extern "C" void DavisExceptionMC(); extern "C" void DavisExceptionXM();
}
