#include "Kernel/Nvme.hpp"
#include <cstdio>
using namespace Davis;
int main(){
 alignas(4096) u64 list[Nvme::MaxPrpListEntries]{}; Nvme::PrpPlan p{};
 if(!Nvme::BuildPrpPlan(0x1000,512,4096,list,512,p)||p.prp2||p.usesList)return 1;
 if(!Nvme::BuildPrpPlan(0x1f00,512,4096,list,512,p)||p.prp2!=0x2000||p.usesList)return 2;
 if(!Nvme::BuildPrpPlan(0x1000,8192,4096,list,512,p)||p.prp2!=0x2000||p.usesList)return 3;
 if(!Nvme::BuildPrpPlan(0x1000,12288,4096,list,512,p)||!p.usesList||p.listEntries!=2||list[0]!=0x2000||list[1]!=0x3000)return 4;
 if(Nvme::BuildPrpPlan(0x1000,(u64)514*4096,4096,list,512,p))return 5;
 unsigned long long cycles=0;
 for(unsigned i=1;i<=100000;i++){u64 off=(i*37u)&4095u;u64 start=0x100000ull+off;u64 bytes=1ull+((i*7919ull)%(128ull*4096ull));if(!Nvme::BuildPrpPlan(start,bytes,4096,list,512,p))return 6;u64 first=4096-off;u64 rem=bytes>first?bytes-first:0;u64 pages=(rem+4095)/4096;if(pages<=1){if(p.usesList)return 7;}else{if(!p.usesList||p.listEntries!=pages)return 8;for(u64 j=0;j<pages;j++)if(list[j]!=(start+first)+j*4096)return 9;}cycles++;}
 u32 stride=4;u32 sq=0,cq=0;if(!Nvme::DoorbellOffsets(stride,0,sq,cq)||sq!=0x1000||cq!=0x1004)return 10;if(!Nvme::DoorbellOffsets(stride,7,sq,cq)||sq!=0x1038||cq!=0x103c)return 11;stride=16;if(!Nvme::DoorbellOffsets(stride,3,sq,cq)||sq!=0x1060||cq!=0x1070)return 12;
 std::printf("PASS nvme_prp cycles=%llu max_entries=%u sq=%x cq=%x\n",cycles,(unsigned)Nvme::MaxPrpListEntries,sq,cq);return 0;
}
