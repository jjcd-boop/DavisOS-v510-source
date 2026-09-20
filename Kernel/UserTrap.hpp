#pragma once
#include "../Base/Types.hpp"
#include "Process.hpp"
namespace Davis::UserTrap {
struct Frame { u64 r15,r14,r13,r12,r11,r10,r9,r8; u64 rbp,rdi,rsi,rdx,rcx,rbx,rax; };
struct State { Process::Image* current; u64 syscallCount; u64 lastNumber; i64 lastValue; u64 lastError; bool exitRequested; u64 capabilityDenials; };
extern State state;
void SetCurrent(Process::Image*);
extern "C" void DavisInt80Dispatch(Frame*);
extern "C" void DavisInt80Entry();
extern "C" [[noreturn]] void DavisEnterUser(u64 rip,u64 rsp,u64 rflags,u64 cs,u64 ss);
}
