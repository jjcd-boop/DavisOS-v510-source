#include "UserMode.hpp"
namespace Davis::UserMode {
extern "C" void DavisReloadSegments(u64,u64);
static u64 seg(u8 access,u8 flags){return 0x000000000000FFFFull | ((u64)access<<40) | ((u64)(flags&0xF)<<52);}
void BuildGdt(GdtState&g,u64 rsp0){g={};
 g.selectors={0x08,0x10,0x1B,0x23,0x28};
 g.entries[1]=seg(0x9A,0xA); // kernel code, long mode
 g.entries[2]=seg(0x92,0x0); // kernel data
 g.entries[3]=seg(0xF2,0x0); // user data DPL3
 g.entries[4]=seg(0xFA,0xA); // user code DPL3
 g.tss.rsp0=rsp0;g.tss.iomapBase=sizeof(Tss64);
 u64 base=(u64)(uptr)&g.tss;u64 limit=sizeof(Tss64)-1;
 g.entries[5]=(limit&0xFFFFull)|((base&0xFFFFFFull)<<16)|(0x89ull<<40)|(((limit>>16)&0xFull)<<48)|(((base>>24)&0xFFull)<<56);
 g.entries[6]=base>>32;g.ready=true;
}
bool Validate(const GdtState&g){return g.ready&&g.selectors.kernelCode==0x08&&g.selectors.userCode==0x23&&g.selectors.userData==0x1B&&g.selectors.tss==0x28&&g.tss.iomapBase==sizeof(Tss64);}
void LoadGdtAndTss(const GdtState&g){
#if defined(__x86_64__) && !defined(DAVIS_HOST_TEST)
 struct GDTR{u16 limit;u64 base;}__attribute__((packed)) gdtr{(u16)(sizeof(g.entries)-1),(u64)(uptr)&g.entries[0]};
 asm volatile("lgdt %0"::"m"(gdtr):"memory");
 // Once GDTR changes, reload the visible segment registers before any IDT gate
 // can rely on Davis-owned selector 0x08. The far return in this assembly
 // helper reloads CS; DS/ES/SS are reloaded with the owned data selector.
 DavisReloadSegments(g.selectors.kernelCode,g.selectors.kernelData);
 u16 ts=g.selectors.tss; asm volatile("ltr %0"::"r"(ts):"memory");
#else
 (void)g;
#endif
}
}
