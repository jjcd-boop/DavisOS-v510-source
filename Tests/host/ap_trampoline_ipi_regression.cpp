#include "../../Kernel/ApTrampoline.hpp"
#include "../../Kernel/ApStartup.hpp"
#include "../../Kernel/LapicIpi.hpp"
#include <cstdio>
using namespace Davis;
struct Mock { u64 icr[8];u32 n;u64 delay;bool fail;};
static bool wr(void*p,u64 v){auto&m=*(Mock*)p;if(m.fail||m.n>=8)return false;m.icr[m.n++]=v;return true;}
static void dl(void*p,u32 us){((Mock*)p)->delay+=us;}
int main(){
 ApTrampoline::Handoff h{}; if(!ApTrampoline::Build(h,3,1,0x200000,0x800000,0xffffffff80001000ull,0x123456789ull,7)||!ApTrampoline::Validate(h))return 1;
 auto bad=h;bad.stackTop^=16;if(ApTrampoline::Validate(bad))return 2;
 if(ApTrampoline::Build(bad,1,1,0x200123,0x800000,1,1,1))return 3;
 u64 plans=0,steps=0;
 for(u32 id=0;id<256;id++)for(u32 vec=1;vec<256;vec++){
  ApStartup::Plan p{};if(!ApStartup::BuildPlan(id,(u64)vec<<12,p))return 4;
  Mock m{};LapicIpi::Transport t{LapicIpi::Mode::XApic,0xfee00000,wr,dl,&m};LapicIpi::Stats s{};
  if(!LapicIpi::ExecutePlan(t,p,s)||s.sent!=4||s.init!=2||s.sipi!=2||m.n!=4||m.delay!=10600)return 5;
  if((m.icr[2]&0xff)!=(u64)vec)return 6;plans++;steps+=s.sent;
 }
 ApStartup::Plan wide{};if(!ApStartup::BuildPlan(0x1234,0x8000,wide))return 7;Mock m{};LapicIpi::Transport xa{LapicIpi::Mode::XApic,0xfee00000,wr,dl,&m};LapicIpi::Stats s{};if(LapicIpi::ExecutePlan(xa,wide,s))return 8;
 LapicIpi::Transport x2{LapicIpi::Mode::X2Apic,0,wr,dl,&m};m={};if(!LapicIpi::ExecutePlan(x2,wide,s)||((m.icr[2]>>32)&0xffffffffu)!=0x1234)return 9;
 m={};m.fail=true;if(LapicIpi::ExecutePlan(x2,wide,s))return 10;
 std::printf("PASS ap_trampoline_ipi plans=%llu steps=%llu handoff_bytes=%zu x2apic=1\n",(unsigned long long)plans,(unsigned long long)steps,sizeof(h));return 0;
}
