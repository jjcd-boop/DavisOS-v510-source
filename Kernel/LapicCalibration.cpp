#include "LapicCalibration.hpp"
namespace Davis::LapicCalibration {
static constexpr u32 LVT=0x320/4,INITIAL=0x380/4,CURRENT=0x390/4,DIVIDE=0x3E0/4;
static constexpr u32 MLVT=0x832,MINITIAL=0x838,MCURRENT=0x839,MDIVIDE=0x83E;
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
Result FromSample(u32 start,u32 end,u64 us,u32 hz){Result r{};r.sampleUs=us;r.startCount=start;r.endCount=end;if(!us||start<=end)return r;u64 elapsed=(u64)start-end;r.timer=TimerCalibration::FromMeasuredTicks(elapsed,us,hz);r.valid=r.timer.valid;return r;}
Result Measure(volatile u32* apic,const TimeReference::State& ref,u64 us,u32 hz,u8 div){Result r{};if(!apic||!ref.usable||us<1000||!hz)return r;u32 oldLvt=apic[LVT],oldDiv=apic[DIVIDE];apic[LVT]=(1u<<16);apic[DIVIDE]=div&0xf;apic[INITIAL]=0xffffffffu;u32 start=apic[CURRENT];if(!TimeReference::WaitMicroseconds(ref,us)){apic[INITIAL]=0;apic[LVT]=oldLvt;apic[DIVIDE]=oldDiv;return r;}u32 end=apic[CURRENT];apic[INITIAL]=0;apic[LVT]=oldLvt;apic[DIVIDE]=oldDiv;return FromSample(start,end,us,hz);}
Result MeasureX2(const TimeReference::State& ref,u64 us,u32 hz,u8 div){Result r{};if(!ref.usable||us<1000||!hz)return r;u64 oldLvt=Rd(MLVT),oldDiv=Rd(MDIVIDE);Wr(MLVT,oldLvt|(1u<<16));Wr(MDIVIDE,div&0xf);Wr(MINITIAL,0xffffffffu);u32 start=(u32)Rd(MCURRENT);if(!TimeReference::WaitMicroseconds(ref,us)){Wr(MINITIAL,0);Wr(MLVT,oldLvt);Wr(MDIVIDE,oldDiv);return r;}u32 end=(u32)Rd(MCURRENT);Wr(MINITIAL,0);Wr(MLVT,oldLvt);Wr(MDIVIDE,oldDiv);return FromSample(start,end,us,hz);}
}
