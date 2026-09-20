#include "NvmeBlockBackend.hpp"
namespace Davis::NvmeBlockBackend {
Binding binding{};
static void copyBytes(u8*d,const u8*s,u64 n){for(u64 i=0;i<n;i++)d[i]=s[i];}
static Transfer* transfer(u16 cid){for(u16 i=0;i<binding.queue.depth;i++)if(binding.transfers[i].active&&binding.transfers[i].cid==cid)return &binding.transfers[i];return nullptr;}
static Transfer* freeTransfer(){for(u16 i=0;i<binding.queue.depth;i++)if(!binding.transfers[i].active)return &binding.transfers[i];return nullptr;}
static void releaseTransfer(Transfer&t,bool copyRead){if(copyRead&&t.op==BlockDevice::Op::Read&&t.user&&t.bounce.virtualAddress)copyBytes((u8*)t.user,(const u8*)t.bounce.virtualAddress,t.bytes);if(binding.allocator){if(t.prpList.physical)Memory::FreeDma(*binding.allocator,t.prpList);if(t.bounce.physical)Memory::FreeDma(*binding.allocator,t.bounce);}t={};}
static bool submit(const BlockDevice::Device&d,BlockDevice::Request&r){
 if(!binding.ready||!binding.controller||d.id!=binding.deviceId||d.backendId!=binding.backendId)return false;
 Nvme::IoOpcode op=Nvme::IoOpcode::Flush;u64 p1=0,p2=0;Transfer*t=nullptr;
 if(r.op==BlockDevice::Op::Read)op=Nvme::IoOpcode::Read;else if(r.op==BlockDevice::Op::Write)op=Nvme::IoOpcode::Write;else if(r.op!=BlockDevice::Op::Flush)return false;
 u64 bytes=(u64)r.sectorCount*d.sectorBytes;
 if(op!=Nvme::IoOpcode::Flush){
  if(!bytes||bytes>MaxBounceBytes||r.sectorCount>65535)return false;
  if(binding.useMmio){
   if(!binding.allocator||(t=freeTransfer())==nullptr)return false;t->bounce=Memory::AllocDma(*binding.allocator,bytes,4096);if(!t->bounce.physical)return false;
   if(op==Nvme::IoOpcode::Write)copyBytes((u8*)t->bounce.virtualAddress,r.buffer,bytes);
   u64 listPages=((bytes+4095)/4096);if(listPages>1){t->prpList=Memory::AllocDma(*binding.allocator,4096,4096);if(!t->prpList.physical){releaseTransfer(*t,false);return false;}}
   Nvme::PrpPlan plan{};u64*list=t->prpList.physical?(u64*)t->prpList.virtualAddress:nullptr;if(!Nvme::BuildPrpPlan(t->bounce.physical,bytes,4096,list,Nvme::MaxPrpListEntries,plan)){releaseTransfer(*t,false);return false;}if(plan.usesList)plan.prp2=t->prpList.physical;p1=plan.prp1;p2=plan.prp2;
  }else{u64 physical=0;if(!binding.mapper||!binding.mapper(r.buffer,bytes,physical))return false;p1=physical;u64 first=4096u-(physical&4095u);if(bytes>first){if(bytes>first+4096u)return false;p2=physical+first;}}
 }
 Nvme::IoCommand cmd{};u16 cid=0;if(!Nvme::SubmitIo(binding.queue,*binding.controller,op,r.lba,(u16)r.sectorCount,p1,p2,cmd,cid)){if(t)releaseTransfer(*t,false);return false;}
 bool published=binding.useMmio?NvmeMmioTransport::Submit(binding.mmioTransport,cmd):Nvme::PublishIo(binding.ring,cmd);if(!published){bool ignored=false;Nvme::CompleteIo(binding.queue,cid,2);Nvme::ReapIo(binding.queue,cid,ignored);if(t)releaseTransfer(*t,false);return false;}
 if(binding.useMmio)NvmeMmioTransport::Ring(binding.mmioTransport,true,binding.mmioTransport->queueId);
 if(t){t->cid=cid;t->op=r.op;t->user=r.buffer;t->bytes=bytes;t->active=true;binding.bounced++;}
 r.backendToken=cid;binding.submitted++;return true;
}
static bool poll(const BlockDevice::Device&,BlockDevice::Request&r,bool&done,bool&success,i32&error){
 done=false;success=false;error=0;if(!binding.ready||!r.backendToken){done=true;error=-20;return true;}
 u16 wanted=(u16)r.backendToken;bool reaped=false;if(Nvme::ReapIo(binding.queue,wanted,reaped)){done=true;success=reaped;if(success)binding.completed++;else{error=-22;binding.failed++;}return true;}
 u16 cid=0,status=0;bool got=false;if(binding.useMmio){got=NvmeMmioTransport::PollAny(binding.mmioTransport,cid,status);if(got)NvmeMmioTransport::Ring(binding.mmioTransport,false,binding.mmioTransport->queueId);}else{bool ok=false;got=Nvme::ConsumeIoCompletion(binding.ring,binding.queue,cid,ok);if(got){done=(cid==wanted);success=ok;if(done){if(ok)binding.completed++;else{error=-22;binding.failed++;}}return true;}}
 if(!got)return true;if(!Nvme::CompleteIo(binding.queue,cid,status)){binding.failed++;return false;}auto*t=transfer(cid);bool ok=Nvme::CompletionSucceeded(status,cid,cid);if(t)releaseTransfer(*t,ok);
 if(cid==wanted&&Nvme::ReapIo(binding.queue,wanted,success)){done=true;if(success)binding.completed++;else{error=-22;binding.failed++;}}return true;
}
static void cancel(const BlockDevice::Device&,BlockDevice::Request&r){if(r.backendToken){u16 cid=(u16)r.backendToken;Nvme::CancelIo(binding.queue,cid);if(auto*t=transfer(cid))releaseTransfer(*t,false);}r.backendToken=0;}
bool Bind(Nvme::State&s,void*sq,void*cq,u16 depth,MapBuffer mapper,u32 backendId,u32 deviceId){if(binding.ready||!s.identifyComplete||!s.namespaceId||!s.namespaceBlocks||!s.logicalBlockBytes||!mapper||!backendId||!deviceId)return false;Binding b{};b.controller=&s;b.mapper=mapper;b.deviceId=deviceId;b.backendId=backendId;if(!Nvme::InitIoQueue(b.queue,depth)||!Nvme::BindIoRing(b.ring,sq,cq,depth))return false;binding=b;if(!BlockDevice::RegisterBackend(backendId,submit,poll,cancel)){binding={};return false;}if(!BlockDevice::RegisterDevice(deviceId,s.namespaceBlocks,s.logicalBlockBytes,true,backendId)){BlockDevice::RemoveBackend(backendId);binding={};return false;}binding.ready=true;return true;}
bool BindRuntime(Nvme::State&s,NvmeStorageRuntime::State&rt,Memory::PageAllocator&a,u32 backendId,u32 deviceId){if(binding.ready||!rt.ready||!rt.ioTransport.bound||!s.identifyComplete||!s.namespaceId||!s.namespaceBlocks||!s.logicalBlockBytes||!backendId||!deviceId)return false;Binding b{};b.controller=&s;b.mmioTransport=&rt.ioTransport;b.allocator=&a;b.deviceId=deviceId;b.backendId=backendId;b.useMmio=true;if(!Nvme::InitIoQueue(b.queue,rt.depth))return false;binding=b;if(!BlockDevice::RegisterBackend(backendId,submit,poll,cancel)){binding={};return false;}if(!BlockDevice::RegisterDevice(deviceId,s.namespaceBlocks,s.logicalBlockBytes,true,backendId)){BlockDevice::RemoveBackend(backendId);binding={};return false;}binding.ready=true;return true;}
void Unbind(){if(!binding.ready)return;BlockDevice::CancelDeviceRequests(binding.deviceId,-30);for(u16 i=0;i<binding.queue.depth;i++)if(binding.transfers[i].active)releaseTransfer(binding.transfers[i],false);BlockDevice::RemoveDevice(binding.deviceId,false);BlockDevice::RemoveBackend(binding.backendId);binding={};}
}
