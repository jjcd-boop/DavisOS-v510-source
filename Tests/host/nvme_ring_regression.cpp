#include "Kernel/Nvme.hpp"
#include <cstdio>
using namespace Davis;
int main(){
 Nvme::State s{};s.identifyComplete=true;s.namespaceId=1;s.namespaceBlocks=1000000;s.logicalBlockBytes=512;
 Nvme::IoQueueState q{}; if(!Nvme::InitIoQueue(q,64)) return 1;
 alignas(4096) Nvme::IoCommand sq[64]{}; alignas(4096) Nvme::IoCompletion cq[64]{}; Nvme::IoRing r{};
 if(!Nvme::BindIoRing(r,sq,cq,64)) return 2;
 Nvme::IoCommand boundary{}; if(!Nvme::BuildIoCommand(s,Nvme::IoOpcode::Read,9,0,17,0x1000,0x2000,boundary)) return 13; if(!Nvme::BuildIoCommand(s,Nvme::IoOpcode::Read,9,0,16,0x1000,0x2000,boundary)) return 14;
 unsigned long long cycles=0; unsigned failures=0;
 for(unsigned i=0;i<100000;i++){
   Nvme::IoCommand cmd{};u16 cid=0; if(!Nvme::SubmitIo(q,s,Nvme::IoOpcode::Read,i%s.namespaceBlocks,1,0x1000,0,cmd,cid))return 3;
   if(!Nvme::PublishIo(r,cmd))return 4;
   unsigned idx=(r.cqHead); cq[idx].cid=cid; cq[idx].status=(u16)((r.cqPhase?1:0) | ((i%997==0)?2:0));
   u16 got=0;bool ok=false;if(!Nvme::ConsumeIoCompletion(r,q,got,ok)||got!=cid)return 5;
   if(i%997==0){if(ok)return 6;failures++;}else if(!ok)return 7;
   cycles++;
 }
 if(q.outstanding||r.sqDoorbells!=cycles||r.cqDoorbells!=cycles)return 8;
 // phase mismatch must not consume; unknown CID must fail closed without advancing CQ.
 Nvme::IoCommand cmd{};u16 cid=0;if(!Nvme::SubmitIo(q,s,Nvme::IoOpcode::Write,7,1,0x2000,0,cmd,cid)||!Nvme::PublishIo(r,cmd))return 9;
 unsigned h=r.cqHead;cq[h].cid=cid;cq[h].status=(u16)(r.cqPhase?0:1);u16 got=0;bool ok=false;if(Nvme::ConsumeIoCompletion(r,q,got,ok)||r.cqHead!=h)return 10;
 cq[h].cid=(u16)(cid+1);cq[h].status=(u16)(r.cqPhase?1:0);if(Nvme::ConsumeIoCompletion(r,q,got,ok)||r.cqHead!=h)return 11;
 cq[h].cid=cid;if(!Nvme::ConsumeIoCompletion(r,q,got,ok)||!ok)return 12;
 std::printf("PASS nvme_ring cycles=%llu failures=%u sqdb=%llu cqdb=%llu phase=%u\n",cycles,failures,(unsigned long long)r.sqDoorbells,(unsigned long long)r.cqDoorbells,(unsigned)r.cqPhase);return 0;
}
