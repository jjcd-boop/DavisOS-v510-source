#include <cstdio>
#include "Kernel/Nvme.hpp"
using namespace Davis;
int main(){
 Nvme::State s{};s.identifyComplete=true;s.namespaceId=1;s.namespaceBlocks=2000000;s.logicalBlockBytes=512;
 Nvme::IoQueueState q{}; if(Nvme::InitIoQueue(q,1)||Nvme::InitIoQueue(q,Nvme::MaxIoQueueDepth+1))return 1;if(!Nvme::InitIoQueue(q,64))return 2;
 Nvme::IoCommand cmd{};u16 cid=0;u16 ids[63]{};
 for(unsigned i=0;i<63;i++){if(!Nvme::SubmitIo(q,s,(i&1)?Nvme::IoOpcode::Read:Nvme::IoOpcode::Write,i*8,8,0x1000,0x2000,cmd,cid))return 3;ids[i]=cid;}
 if(Nvme::SubmitIo(q,s,Nvme::IoOpcode::Read,0,1,0x1000,0,cmd,cid))return 4;
 if(Nvme::CompleteIo(q,65000,1))return 5;
 for(unsigned i=0;i<63;i++){if(!Nvme::CompleteIo(q,ids[i],1))return 6;bool ok=false;if(!Nvme::ReapIo(q,ids[i],ok)||!ok)return 7;}
 if(q.outstanding)return 8;
 unsigned long long cycles=0;for(unsigned i=0;i<200000;i++){u64 lba=(i*131ull)%1999000ull;if(!Nvme::SubmitIo(q,s,(i&1)?Nvme::IoOpcode::Read:Nvme::IoOpcode::Write,lba,1+(i%16),0x1000,0x2000,cmd,cid))return 9;u16 status=(i%997)==0?3:1;if(!Nvme::CompleteIo(q,cid,status))return 10;bool ok=true;if(!Nvme::ReapIo(q,cid,ok))return 11;if(ok!=(status==1))return 12;cycles++;}
 if(q.outstanding||q.submitted!=200063ull||q.completed+q.failed!=q.submitted)return 13;
 std::printf("PASS nvme_queue cycles=%llu submitted=%llu completed=%llu failed=%llu depth=%u\n",cycles,(unsigned long long)q.submitted,(unsigned long long)q.completed,(unsigned long long)q.failed,q.depth);return 0;
}
