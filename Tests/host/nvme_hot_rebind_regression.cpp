#include "Kernel/NvmeBlockBackend.hpp"
#include <cstdio>
using namespace Davis;
static bool mapbuf(void* v,u64,u64& p){p=(u64)(uptr)v;return v!=nullptr;}
int main(){
 BlockDevice::Init(); Nvme::State c{}; c.identifyComplete=true;c.namespaceId=1;c.namespaceBlocks=100000;c.logicalBlockBytes=512;
 alignas(4096) Nvme::IoCommand sq[8]{}; alignas(4096) Nvme::IoCompletion cq[8]{}; u8 buf[512]{};
 u64 cycles=10000,cancelled=0;
 for(u64 n=0;n<cycles;n++){
  if(!NvmeBlockBackend::Bind(c,sq,cq,8,mapbuf,2,2)) return 1;
  u64 id=BlockDevice::Submit(2,BlockDevice::Op::Write,n%99999,1,buf,100); if(!id)return 2;
  BlockDevice::PumpOnce(); if(BlockDevice::Query(id)!=BlockDevice::Status::Active)return 3;
  NvmeBlockBackend::Unbind();
  i32 e=0;if(BlockDevice::Query(id,&e)!=BlockDevice::Status::Failed||e!=-30)return 4; cancelled++;
  if(BlockDevice::GetDevice(2)!=nullptr)return 5;
  if(!BlockDevice::Reap(id,&e))return 6;
 }
 if(!NvmeBlockBackend::Bind(c,sq,cq,8,mapbuf,2,2))return 7;NvmeBlockBackend::Unbind();
 std::printf("PASS nvme_hot_rebind cycles=%llu cancelled=%llu backend_reuse=1 device_reuse=1\n",(unsigned long long)cycles,(unsigned long long)cancelled);return 0;
}
