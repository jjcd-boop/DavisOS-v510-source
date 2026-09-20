#include "ForgePersistentStore.hpp"
namespace Davis::ForgePersistentStore {
struct Guid{u32 a;u16 b,c;u8 d[8];};
struct Hdr{u64 sig;u32 rev,size,crc,res;};
struct RuntimeServices{Hdr hdr;void*getTime;void*setTime;void*getWakeupTime;void*setWakeupTime;void*setVirtualAddressMap;void*convertPointer;u64(__attribute__((ms_abi))*getVariable)(u16*,Guid*,u32*,usize*,void*);void*getNextVariableName;u64(__attribute__((ms_abi))*setVariable)(u16*,Guid*,u32,usize,void*);};
static RuntimeServices* rt=nullptr;
static Guid guid{0x44415649,0x5346,0x4f52,{0x47,0x45,0x53,0x54,0x4f,0x52,0x45,0x31}};
void Init(const BootInfo&b){rt=(RuntimeServices*)b.runtimeServices;}
bool Available(){return rt&&rt->getVariable&&rt->setVariable;}
static void name(u32 k,u16 out[24]){const char*p="DavisForgeState";u32 n=0;while(p[n]){out[n]=(u16)p[n];n++;}const char*h="0123456789ABCDEF";for(int s=28;s>=0;s-=4)out[n++]=(u16)h[(k>>s)&15];out[n]=0;}
i64 Read(u32 k,void*d,u64 bytes){if(!Available()||!k||!d||!bytes||bytes>512)return -1;u16 nm[24];name(k,nm);usize n=(usize)bytes;u32 attrs=0;u64 r=rt->getVariable(nm,&guid,&attrs,&n,d);return r==0?(i64)n:0;}
i64 Write(u32 k,const void*s,u64 bytes){if(!Available()||!k||!s||!bytes||bytes>512)return -1;u16 nm[24];name(k,nm);constexpr u32 attrs=1|2|4;u64 r=rt->setVariable(nm,&guid,attrs,(usize)bytes,(void*)s);return r==0?(i64)bytes:-1;}
}
