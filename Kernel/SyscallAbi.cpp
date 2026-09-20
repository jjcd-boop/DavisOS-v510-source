#include "SyscallAbi.hpp"
namespace Davis::SyscallAbi {
Syscall::Frame Decode(const Registers&r){return {r.rax,r.rdi,r.rsi,r.rdx,r.r10,r.r8,r.r9};}
void Encode(const Syscall::Result&x,Registers&r){r.rax=(u64)x.value;r.rdx=x.error;}
}
