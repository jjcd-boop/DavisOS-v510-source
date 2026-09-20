#include "TimeReference.hpp"
namespace Davis::TimeReference {
State Decode(u32 maxLeaf,u32 denom,u32 numer,u32 crystal,u32 baseMHz,bool invariant){
 State s{};s.invariant=invariant;
 if(maxLeaf>=0x15 && denom && numer && crystal){s.source=Source::Cpuid15Tsc;s.tscHz=((u64)crystal*numer)/denom;}
 else if(maxLeaf>=0x16 && baseMHz){s.source=Source::Cpuid16Tsc;s.tscHz=(u64)baseMHz*1000000ull;}
 s.usable=s.tscHz>=1000000ull && invariant;return s;
}
u64 ReadTsc(){
#if defined(__x86_64__) && !defined(DAVIS_HOST_TEST)
 u32 lo,hi;asm volatile("lfence; rdtsc":"=a"(lo),"=d"(hi)::"memory");return ((u64)hi<<32)|lo;
#else
 return 0;
#endif
}
State Probe(){
#if defined(__x86_64__) && !defined(DAVIS_HOST_TEST)
 u32 a=0,b=0,c=0,d=0;asm volatile("cpuid":"+a"(a),"=b"(b),"=c"(c),"=d"(d));u32 max=a;
 bool inv=false;a=0x80000000;b=c=d=0;asm volatile("cpuid":"+a"(a),"=b"(b),"=c"(c),"=d"(d));if(a>=0x80000007){a=0x80000007;asm volatile("cpuid":"+a"(a),"=b"(b),"=c"(c),"=d"(d));inv=(d&(1u<<8))!=0;}
 u32 den=0,num=0,cr=0,mhz=0;if(max>=0x15){a=0x15;b=c=d=0;asm volatile("cpuid":"+a"(a),"=b"(b),"=c"(c),"=d"(d));den=a;num=b;cr=c;}if(max>=0x16){a=0x16;b=c=d=0;asm volatile("cpuid":"+a"(a),"=b"(b),"=c"(c),"=d"(d));mhz=a;}return Decode(max,den,num,cr,mhz,inv);
#else
 return {};
#endif
}
bool WaitMicroseconds(const State&s,u64 us){if(!s.usable||!us)return false;u64 span=(s.tscHz/1000000ull)*us;if(!span)return false;u64 start=ReadTsc();while(ReadTsc()-start<span)asm volatile("pause");return true;}
}
