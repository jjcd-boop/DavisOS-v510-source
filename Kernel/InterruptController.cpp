#include "InterruptController.hpp"
namespace Davis::InterruptController {
static State* g=nullptr;
static inline void Wrmsr(u32 msr,u64 v){
#if defined(__x86_64__) && !defined(DAVIS_HOST_TEST)
 u32 lo=(u32)v,hi=(u32)(v>>32);asm volatile("wrmsr"::"c"(msr),"a"(lo),"d"(hi):"memory");
#else
 (void)msr;(void)v;
#endif
}
void InitNone(State&s){s={Kind::None,nullptr,0,false};}
bool InitLocalApic(State&s,u64 base){if(!base || (base&0xfffull))return false;s.kind=Kind::LocalApic;s.lapic=(volatile u32*)base;s.eoiCount=0;s.ready=true;return true;}
bool InitX2Apic(State&s){s.kind=Kind::X2Apic;s.lapic=nullptr;s.eoiCount=0;s.ready=true;return true;}
void EndOfInterrupt(State&s){if(!s.ready)return;if(s.kind==Kind::LocalApic&&s.lapic)s.lapic[0xB0/4]=0;else if(s.kind==Kind::X2Apic)Wrmsr(0x80B,0);else return;s.eoiCount++;}
void SetGlobal(State*s){g=s;}
extern "C" void DavisTimerEoi(){if(g)EndOfInterrupt(*g);}
}
