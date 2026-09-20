#include "Kernel/ApStartup.hpp"
#include "Kernel/SmpTopology.hpp"
#include <cstdio>
using namespace Davis;
int main(){
 ApStartup::Plan p{};if(!ApStartup::BuildPlan(7,0x8000,p)||!p.valid||p.count!=4)return 1;
 if(p.steps[0].kind!=ApStartup::Kind::InitAssert||p.steps[1].kind!=ApStartup::Kind::InitDeassert)return 2;
 if(p.steps[2].vector!=8||p.steps[3].vector!=8)return 3;
 if(((p.steps[2].icr>>32)&0xffffffffu)!=7||((p.steps[2].icr>>8)&7u)!=6u)return 4;
 if(ApStartup::BuildPlan(1,0x8123,p)||ApStartup::BuildPlan(1,0x100000,p)||ApStartup::BuildPlan(1,0,p))return 5;
 for(u32 id=0;id<64;id++)for(u32 page=1;page<256;page++){if(!ApStartup::BuildPlan(id,(u64)page<<12,p))return 6;if(p.steps[2].vector!=page)return 7;}
 std::printf("PASS ap_startup plans=%u trampoline_range=0x1000..0xff000 retries=%u\n",64u*255u,SmpTopology::MaxStartupAttempts);return 0;
}
