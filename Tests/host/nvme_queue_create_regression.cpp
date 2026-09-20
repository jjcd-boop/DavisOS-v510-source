#include "Kernel/Nvme.hpp"
#include <cstdio>
using namespace Davis;
int main(){
 Nvme::QueueCreatePlan p{};if(!Nvme::BuildQueueCreatePlan(1,64,0x200000,0x210000,5,p)||!p.valid)return 1;
 if(p.createCq.opcode!=0x05||p.createSq.opcode!=0x01)return 2;
 if((p.createCq.cdw10&0xffffu)!=1||((p.createCq.cdw10>>16)&0xffffu)!=63)return 3;
 if(((p.createCq.cdw11>>16)&0xffffu)!=5||(p.createCq.cdw11&3u)!=3u)return 4;
 if(((p.createSq.cdw11>>16)&0xffffu)!=1||(p.createSq.cdw11&1u)!=1u)return 5;
 if(Nvme::BuildQueueCreatePlan(0,64,0x200000,0x210000,0,p))return 6;
 if(Nvme::BuildQueueCreatePlan(1,1,0x200000,0x210000,0,p))return 7;
 if(Nvme::BuildQueueCreatePlan(1,Nvme::MaxIoQueueDepth+1,0x200000,0x210000,0,p))return 8;
 if(Nvme::BuildQueueCreatePlan(1,64,0x200001,0x210000,0,p))return 9;
 for(u32 i=0;i<100000;i++){u16 d=(u16)(2+(i%(Nvme::MaxIoQueueDepth-1)));u16 q=(u16)(1+(i%31));u16 v=(u16)(i%128);if(!Nvme::BuildQueueCreatePlan(q,d,0x400000ull+((u64)(i%16)<<12),0x500000ull+((u64)(i%16)<<12),v,p))return 10;if(p.depth!=d||p.queueId!=q)return 11;}
 std::printf("PASS nvme_queue_create plans=100000 max_depth=%u\n",Nvme::MaxIoQueueDepth);return 0;
}
