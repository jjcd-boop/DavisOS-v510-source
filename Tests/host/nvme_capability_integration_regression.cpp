#include "Kernel/NvmeControllerTransport.hpp"
#include "Kernel/NvmeMmioTransport.hpp"
#include <cstdio>
using namespace Davis;
static u64 cap(u16 mqesMinus1,u8 dstrd,u8 mpsmin,u8 mpsmax,bool nvm=true){return (u64)mqesMinus1|((u64)dstrd<<32)|((u64)(nvm?1:0)<<37)|((u64)mpsmin<<48)|((u64)mpsmax<<52);}
int main(){
 alignas(4096) unsigned char mmio[0x9000]{};alignas(64) Nvme::IoCommand sq[64]{};alignas(64) Nvme::IoCompletion cq[64]{};
 u64 cycles=0;
 for(u32 n=0;n<100000;n++){
  u8 ds=(u8)(n%5);*(u64*)(mmio)=cap(63,ds,0,0,true);*(u32*)(mmio+0x1c)=0;
  NvmeControllerTransport::State c{};if(!NvmeControllerTransport::Bind(c,mmio,4096))return 1;
  if(c.capabilities.doorbellStrideBytes!=(4u<<ds)||c.capabilities.maxQueueEntries!=64)return 2;
  if(!NvmeControllerTransport::Disable(c,2)||!NvmeControllerTransport::ConfigureAdmin(c,0x100000,0x110000,64))return 3;
  *(u32*)(mmio+0x1c)=1;if(!NvmeControllerTransport::Enable(c,2))return 4;
  NvmeMmioTransport::State q{};if(!NvmeMmioTransport::BindFromController(q,c,sq,cq,64,0))return 5;
  if(q.doorbellStride!=(4u<<ds))return 6;cycles++;
 }
 NvmeControllerTransport::State bad{};*(u64*)mmio=cap(63,0,0,0,false);if(NvmeControllerTransport::Bind(bad,mmio,4096))return 7;
 *(u64*)mmio=cap(7,0,0,0,true);*(u32*)(mmio+0x1c)=0;if(!NvmeControllerTransport::Bind(bad,mmio,4096)||!NvmeControllerTransport::Disable(bad,2))return 8;if(NvmeControllerTransport::ConfigureAdmin(bad,0x100000,0x110000,64))return 9;
 *(u64*)mmio=cap(63,0,1,1,true);if(NvmeControllerTransport::Bind(bad,mmio,4096))return 10;
 std::printf("PASS nvme_capability_integration cycles=%llu css_reject=1 mqes_reject=1 mps_reject=1\n",(unsigned long long)cycles);
}
