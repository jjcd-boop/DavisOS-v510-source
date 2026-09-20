#pragma once
#include "Syscall.hpp"
namespace Davis::SyscallAbi {
static constexpr u8 Vector=0x80;
struct Registers { u64 rax,rdi,rsi,rdx,r10,r8,r9; };
Syscall::Frame Decode(const Registers&);
void Encode(const Syscall::Result&,Registers&);
}
