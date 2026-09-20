#include "ApStartup.hpp"
namespace Davis::ApStartup {
bool ValidateTrampoline(u64 p){return p>=0x1000ull&&p<0x100000ull&&(p&0xfffull)==0;}
static u32 low(Kind k,u8 v){
 switch(k){
  case Kind::InitAssert:return (5u<<8)|(1u<<14)|(1u<<15);
  case Kind::InitDeassert:return (5u<<8)|(1u<<15);
  case Kind::Startup:case Kind::StartupRetry:return (6u<<8)|(u32)v;
 }
 return 0;
}
u64 EncodeX2ApicIcr(Kind k,u32 id,u8 v){return ((u64)id<<32)|low(k,v);}
u64 EncodeXApicIcr(Kind k,u32 id,u8 v){return ((u64)(id&0xffu)<<56)|low(k,v);}
bool BuildPlan(u32 id,u64 tramp,Plan&p){p={};if(!ValidateTrampoline(tramp))return false;u64 vec=tramp>>12;if(vec>0xffu)return false;p.trampolinePhysical=tramp;p.count=4;p.valid=true;
 p.steps[0]={Kind::InitAssert,id,0,10000,EncodeX2ApicIcr(Kind::InitAssert,id,0)};
 p.steps[1]={Kind::InitDeassert,id,0,200,EncodeX2ApicIcr(Kind::InitDeassert,id,0)};
 p.steps[2]={Kind::Startup,id,(u8)vec,200,EncodeX2ApicIcr(Kind::Startup,id,(u8)vec)};
 p.steps[3]={Kind::StartupRetry,id,(u8)vec,200,EncodeX2ApicIcr(Kind::StartupRetry,id,(u8)vec)};return true;}
}
