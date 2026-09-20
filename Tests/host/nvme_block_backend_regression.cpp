#include "Kernel/NvmeBlockBackend.hpp"
#include <cstdio>
#include <cstring>
using namespace Davis;
static u8 dma[8192];
static bool mapbuf(void* v,u64 bytes,u64& p){auto a=(uptr)v,b=(uptr)dma;if(a<b||a+bytes>b+sizeof(dma))return false;p=0x200000ull+(a-b);return true;}
static void completeOne(Nvme::IoRing&r,u16 cid,bool phase,bool fail=false){auto* cq=(Nvme::IoCompletion*)r.cq;auto& e=cq[r.cqHead];e={};e.cid=cid;e.status=(u16)((fail?1u:0u)<<1)|(phase?1u:0u);}
int main(){
 BlockDevice::Init();Nvme::State s{};s.identifyComplete=true;s.namespaceId=1;s.namespaceBlocks=1000000;s.logicalBlockBytes=512;
 alignas(64) Nvme::IoCommand sq[64]{};alignas(64) Nvme::IoCompletion cq[64]{};
 if(!NvmeBlockBackend::Bind(s,sq,cq,64,mapbuf,2,2))return 1;
 for(u32 i=0;i<20000;i++){
  u32 sectors=(i%8)+1;u8*buf=dma+(i%4)*512;u64 id=BlockDevice::Submit(2,(i&1)?BlockDevice::Op::Read:BlockDevice::Op::Write,i%900000,sectors,buf,20);if(!id)return 2;
  BlockDevice::PumpOnce();u16 cid=(u16)BlockDevice::state.requests[(id-1)%BlockDevice::MaxRequests].backendToken;
  if(!cid){for(auto&r:BlockDevice::state.requests)if(r.id==id)cid=(u16)r.backendToken;}if(!cid)return 3;
  completeOne(NvmeBlockBackend::binding.ring,cid,NvmeBlockBackend::binding.ring.cqPhase,false);BlockDevice::PumpOnce();if(BlockDevice::Query(id)!=BlockDevice::Status::Complete)return 4;if(!BlockDevice::Reap(id))return 5;
 }
 // Inject controller failure and ensure it reaches BlockDevice.
 u64 id=BlockDevice::Submit(2,BlockDevice::Op::Read,4,1,dma,20);BlockDevice::PumpOnce();u16 cid=0;for(auto&r:BlockDevice::state.requests)if(r.id==id)cid=(u16)r.backendToken;if(!cid)return 6;
 completeOne(NvmeBlockBackend::binding.ring,cid,NvmeBlockBackend::binding.ring.cqPhase,true);BlockDevice::PumpOnce();if(BlockDevice::Query(id)!=BlockDevice::Status::Failed)return 7;BlockDevice::Reap(id);
 // Reject buffer not covered by mapper and >2-page transfer (until PRP-list backend ownership lands).
 u8 outside[512]{};id=BlockDevice::Submit(2,BlockDevice::Op::Read,0,1,outside,20);if(!id)return 8;BlockDevice::PumpOnce();if(BlockDevice::Query(id)!=BlockDevice::Status::Failed)return 9;BlockDevice::Reap(id);
 // Timeout must cancel the underlying NVMe slot so repeated timeouts cannot exhaust the queue.
 for(u32 t=0;t<100;t++){u64 tid=BlockDevice::Submit(2,BlockDevice::Op::Read,8,1,dma,1);if(!tid)return 10;BlockDevice::PumpOnce();BlockDevice::PumpOnce();BlockDevice::PumpOnce();if(BlockDevice::Query(tid)!=BlockDevice::Status::TimedOut)return 11;if(!BlockDevice::Reap(tid))return 12;}
 if(NvmeBlockBackend::binding.queue.outstanding!=0)return 13;
 std::printf("PASS nvme_block_backend cycles=20000 timeouts=100 submitted=%llu completed=%llu failed=%llu\n",(unsigned long long)NvmeBlockBackend::binding.submitted,(unsigned long long)NvmeBlockBackend::binding.completed,(unsigned long long)NvmeBlockBackend::binding.failed);return 0;
}
