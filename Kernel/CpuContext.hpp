#pragma once
#include "../Base/Types.hpp"
namespace Davis::CpuContext {
// Canonical interrupt context used by the preemption layer. Assembly layout is fixed.
struct Frame {
 u64 r15,r14,r13,r12,r11,r10,r9,r8;
 u64 rbp,rdi,rsi,rdx,rcx,rbx,rax;
 u64 vector,error;
 u64 rip,cs,rflags,rsp,ss;
};
static_assert(sizeof(Frame)==22*8,"CpuContext::Frame ABI changed");
inline bool FromUser(const Frame& f){return (f.cs&3u)==3u;}
inline bool CanonicalRip(const Frame& f){u64 x=f.rip;u64 hi=x>>48;return hi==0 || hi==0xffff;}
}
