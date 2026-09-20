#include "NvmeStorageRuntime.hpp"
namespace Davis::NvmeStorageRuntime {
static void fail(State&s,Memory::PageAllocator&a){if(s.ioSq.physical)Memory::FreeDma(a,s.ioSq);if(s.ioCq.physical)Memory::FreeDma(a,s.ioCq);s.ready=false;s.stage=Stage::Failed;s.failures++;}
bool BringUp(State&s,Nvme::State&c,Memory::PageAllocator&a,u16 depth,u16 qid,u32 limit){
 s={};s.queueId=qid;s.depth=depth;
 if(!c.adminReady||!c.identifyComplete||!c.adminSq.virtualAddress||!c.adminCq.virtualAddress||depth<2||depth>Nvme::MaxIoQueueDepth||depth>c.maxQueueEntries||!qid||limit<100||limit>50000000){s.stage=Stage::Failed;s.failures++;return false;}
 s.ioSq=Memory::AllocDma(a,(u64)depth*sizeof(Nvme::IoCommand),4096);s.ioCq=Memory::AllocDma(a,(u64)depth*sizeof(Nvme::IoCompletion),4096);
 if(!s.ioSq.physical||!s.ioCq.physical){fail(s,a);return false;}Memory::Zero(s.ioSq);Memory::Zero(s.ioCq);s.stage=Stage::Allocated;
 if(!NvmeMmioTransport::Bind(s.adminTransport,(void*)(uptr)c.mmio,c.adminSq.virtualAddress,c.adminCq.virtualAddress,c.adminDepth,c.doorbellStrideBytes,0)){fail(s,a);return false;}s.adminTransport.sqTail=c.adminSqTail;s.adminTransport.cqHead=c.adminCqHead;s.adminTransport.cqPhase=c.adminCqPhase;s.stage=Stage::AdminBound;
 Nvme::QueueCreatePlan p{};if(!Nvme::BuildQueueCreatePlan(qid,depth,s.ioSq.physical,s.ioCq.physical,0,p)){fail(s,a);return false;}
 NvmeAdminRuntime::Transport t{NvmeMmioTransport::Submit,NvmeMmioTransport::Poll,NvmeMmioTransport::Ring,NvmeMmioTransport::Reset,&s.adminTransport};
 u16 cqCid=c.nextCommandId++;if(!c.nextCommandId)c.nextCommandId=1;u16 sqCid=c.nextCommandId++;if(!c.nextCommandId)c.nextCommandId=1;
 if(!NvmeAdminRuntime::CreateIoQueues(t,p,cqCid,sqCid,limit,s.adminRuntime)){fail(s,a);return false;}
 c.adminSqTail=s.adminTransport.sqTail;c.adminCqHead=s.adminTransport.cqHead;c.adminCqPhase=s.adminTransport.cqPhase;s.stage=Stage::QueuesCreated;if(!NvmeMmioTransport::Bind(s.ioTransport,(void*)(uptr)c.mmio,s.ioSq.virtualAddress,s.ioCq.virtualAddress,depth,c.doorbellStrideBytes,qid)){fail(s,a);return false;}s.ready=true;s.bringups++;s.stage=Stage::Ready;return true;
}
void Shutdown(State&s,Memory::PageAllocator&a){if(s.ioSq.physical)Memory::FreeDma(a,s.ioSq);if(s.ioCq.physical)Memory::FreeDma(a,s.ioCq);s.ready=false;s.stage=Stage::Cold;}
}
