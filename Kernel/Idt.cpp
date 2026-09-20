#include "Idt.hpp"
namespace Davis::Idt {
void Init(Table&t){t={};}
void SetInterruptGate(Table&t,u8 v,void(*h)(),u16 sel,u8 dpl,u8 ist){u64 a=(u64)(uptr)h;auto&g=t.gate[v];g.off0=a&0xffff;g.selector=sel;g.ist=ist&7;g.typeAttr=(u8)(0x8E|((dpl&3)<<5));g.off1=(a>>16)&0xffff;g.off2=(u32)(a>>32);g.reserved=0;t.ready=true;}
bool ValidateSyscallGate(const Table&t,u8 v,u16 kc){auto&g=t.gate[v];return t.ready&&g.selector==kc&&(g.typeAttr&0x80)&&((g.typeAttr>>5)&3)==3&&(g.typeAttr&0x0f)==0x0e;}
void Load(const Table&t){
#if defined(__x86_64__) && !defined(DAVIS_HOST_TEST)
 struct IDTR{u16 limit;u64 base;}__attribute__((packed)) r{(u16)(sizeof(t.gate)-1),(u64)(uptr)&t.gate[0]};asm volatile("lidt %0"::"m"(r):"memory");
#else
 (void)t;
#endif
}
}
