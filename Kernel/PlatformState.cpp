#include "PlatformState.hpp"
namespace Davis::PlatformState {
Snapshot Capture(const UserMode::GdtState&g){Snapshot s{};
#if defined(__x86_64__) && !defined(DAVIS_HOST_TEST)
 asm volatile("mov %%cs,%0":"=r"(s.cs)); asm volatile("mov %%ss,%0":"=r"(s.ss)); asm volatile("mov %%ds,%0":"=r"(s.ds));
 asm volatile("str %0":"=r"(s.tr)); asm volatile("mov %%cr3,%0":"=r"(s.cr3));
#else
 s.cs=g.selectors.kernelCode;s.ss=g.selectors.kernelData;s.ds=g.selectors.kernelData;s.tr=g.selectors.tss;s.cr3=0x1000;
#endif
 s.selectorsOwned=s.cs==g.selectors.kernelCode&&s.ss==g.selectors.kernelData&&s.ds==g.selectors.kernelData;
 s.tssLoaded=(s.tr&~3u)==g.selectors.tss;s.pagingActive=(s.cr3&~0xFFFull)!=0;return s;}
bool ValidateOwned(const Snapshot&s,const UserMode::GdtState&g){return g.ready&&s.selectorsOwned&&s.tssLoaded&&s.pagingActive;}
}
