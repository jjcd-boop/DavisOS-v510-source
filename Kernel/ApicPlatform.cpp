#include "ApicPlatform.hpp"
namespace Davis::ApicPlatform {
State Decode(u32 edx,u64 msr){State s{};s.cpuidSupported=true;s.localApicPresent=(edx&ApicFeatureBit)!=0;s.msrEnabled=(msr&(1ull<<11))!=0;s.x2Apic=(msr&(1ull<<10))!=0;s.physicalBase=msr&0x000ffffffffff000ull;s.usable=s.localApicPresent&&s.msrEnabled&&s.physicalBase&&((s.physicalBase&0xfffull)==0);return s;}
State Probe(){
#if defined(__x86_64__) && !defined(DAVIS_HOST_TEST)
 u32 a=1,b=0,c=0,d=0;asm volatile("cpuid":"+a"(a),"=b"(b),"=c"(c),"=d"(d));if(!(d&ApicFeatureBit))return Decode(d,0);u32 lo=0,hi=0;asm volatile("rdmsr":"=a"(lo),"=d"(hi):"c"(ApicBaseMsr));return Decode(d,((u64)hi<<32)|lo);
#else
 return {};
#endif
}
}
