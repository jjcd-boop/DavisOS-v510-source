#include "KernelSecurity.hpp"
namespace Davis::KernelSecurity {
State state{};
static inline void cpuid(u32 leaf,u32 sub,u32&a,u32&b,u32&c,u32&d){asm volatile("cpuid":"=a"(a),"=b"(b),"=c"(c),"=d"(d):"a"(leaf),"c"(sub));}
static inline u64 rdmsr(u32 m){u32 a,d;asm volatile("rdmsr":"=a"(a),"=d"(d):"c"(m));return ((u64)d<<32)|a;}
static inline void wrmsr(u32 m,u64 v){u32 a=(u32)v,d=(u32)(v>>32);asm volatile("wrmsr"::"c"(m),"a"(a),"d"(d):"memory");}
[[noreturn]] void FatalIntegrityFault(){asm volatile("cli":::"memory");for(;;)asm volatile("hlt");}
void Init(){
 state={};u32 a=0,b=0,c=0,d=0;cpuid(0,0,a,b,c,d);state.cpuid=true;state.maxBasicLeaf=a;
 if(state.maxBasicLeaf>=7){cpuid(7,0,a,b,c,d);state.smepSupported=(b&(1u<<7))!=0;state.smapSupported=(b&(1u<<20))!=0;}
 cpuid(0x80000000u,0,a,b,c,d);state.maxExtendedLeaf=a;if(a>=0x80000001u){cpuid(0x80000001u,0,a,b,c,d);state.nxSupported=(d&(1u<<20))!=0;}
 u64 cr0=0;asm volatile("mov %%cr0,%0":"=r"(cr0));cr0|=(1ull<<16);asm volatile("mov %0,%%cr0"::"r"(cr0):"memory");u64 verify=0;asm volatile("mov %%cr0,%0":"=r"(verify));state.writeProtectEnabled=(verify&(1ull<<16))!=0;
 if(state.nxSupported){u64 efer=rdmsr(0xC0000080u);efer|=(1ull<<11);wrmsr(0xC0000080u,efer);state.nxEnabled=(rdmsr(0xC0000080u)&(1ull<<11))!=0;}
 state.ready=state.writeProtectEnabled&&(!state.nxSupported||state.nxEnabled);if(!state.ready)FatalIntegrityFault();
}
}
extern "C" {
Davis::uptr __stack_chk_guard=(Davis::uptr)0x9e3779b97f4a7c15ull;
[[noreturn]] void __stack_chk_fail(){Davis::KernelSecurity::FatalIntegrityFault();}
}
