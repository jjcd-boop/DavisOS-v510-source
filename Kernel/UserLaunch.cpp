#include "UserLaunch.hpp"
namespace Davis::UserLaunch {
bool Build(Frame&f,u64 e,u64 s,u64 p,const UserMode::GdtState&g){f={};if(!g.ready||!e||!s||!p||(s&15))return false;f.rip=e;f.rsp=s;f.rflags=0x202;f.cr3=p;f.cs=g.selectors.userCode;f.ss=g.selectors.userData;f.valid=true;return true;}
u64 ReadCr3(){
#if defined(__x86_64__) && !defined(DAVIS_HOST_TEST)
 u64 v;asm volatile("mov %%cr3,%0":"=r"(v));return v;
#else
 return 0;
#endif
}
void WriteCr3(u64 p){
#if defined(__x86_64__) && !defined(DAVIS_HOST_TEST)
 asm volatile("mov %0,%%cr3"::"r"(p):"memory");
#else
 (void)p;
#endif
}
}
