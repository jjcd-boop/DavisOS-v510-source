#include "LocalApicTimer.hpp"
namespace Davis::LocalApicTimer {
static constexpr u32 REG_LVT_TIMER=0x320/4,REG_INITIAL=0x380/4,REG_CURRENT=0x390/4,REG_DIVIDE=0x3E0/4;
static constexpr u32 MSR_LVT_TIMER=0x832,MSR_INITIAL=0x838,MSR_CURRENT=0x839,MSR_DIVIDE=0x83E;
static inline void Wr(u32 m,u64 v){
#if defined(__x86_64__) && !defined(DAVIS_HOST_TEST)
 u32 lo=(u32)v,hi=(u32)(v>>32);asm volatile("wrmsr"::"c"(m),"a"(lo),"d"(hi):"memory");
#else
 (void)m;(void)v;
#endif
}
static inline u64 Rd(u32 m){
#if defined(__x86_64__) && !defined(DAVIS_HOST_TEST)
 u32 lo,hi;asm volatile("rdmsr":"=a"(lo),"=d"(hi):"c"(m));return ((u64)hi<<32)|lo;
#else
 (void)m;return 0;
#endif
}
void Init(State&s){s={};s.vector=DefaultVector;s.divideCode=0x3;s.mode=Mode::Disabled;s.backend=Backend::None;}
bool ConfigurePeriodic(State&s,u64 base,u8 vector,u32 count,u8 divide){if(!base||(base&0xfffull)||vector<0x20||!count)return false;s.lapic=(volatile u32*)(uptr)base;s.vector=vector;s.ticksPerQuantum=count;s.divideCode=divide&0xf;s.mode=Mode::Periodic;s.backend=Backend::XApicMmio;s.configured=true;s.armed=false;return true;}
bool ConfigurePeriodicX2(State&s,u8 vector,u32 count,u8 divide){if(vector<0x20||!count)return false;s.lapic=nullptr;s.vector=vector;s.ticksPerQuantum=count;s.divideCode=divide&0xf;s.mode=Mode::Periodic;s.backend=Backend::X2ApicMsr;s.configured=true;s.armed=false;return true;}
void Arm(State&s){if(!s.configured)return;if(s.backend==Backend::XApicMmio&&s.lapic){s.lapic[REG_DIVIDE]=s.divideCode;s.lapic[REG_LVT_TIMER]=(u32)s.vector|(1u<<17);s.lapic[REG_INITIAL]=s.ticksPerQuantum;}else if(s.backend==Backend::X2ApicMsr){Wr(MSR_DIVIDE,s.divideCode);Wr(MSR_LVT_TIMER,(u32)s.vector|(1u<<17));Wr(MSR_INITIAL,s.ticksPerQuantum);}else return;s.armed=true;}
void Stop(State&s){if(s.backend==Backend::XApicMmio&&s.lapic){s.lapic[REG_LVT_TIMER]|=(1u<<16);s.lapic[REG_INITIAL]=0;}else if(s.backend==Backend::X2ApicMsr){Wr(MSR_LVT_TIMER,Rd(MSR_LVT_TIMER)|(1u<<16));Wr(MSR_INITIAL,0);}s.armed=false;}
void NoteInterrupt(State&s){if(s.configured)++s.interrupts;}
u32 CurrentCount(const State&s){if(!s.configured)return 0;if(s.backend==Backend::XApicMmio&&s.lapic)return s.lapic[REG_CURRENT];if(s.backend==Backend::X2ApicMsr)return (u32)Rd(MSR_CURRENT);return 0;}
bool Validate(const State&s){return s.configured&&s.backend!=Backend::None&&s.vector>=0x20&&s.ticksPerQuantum&&s.mode==Mode::Periodic&&((s.backend==Backend::X2ApicMsr)||(s.lapic!=nullptr));}
}
