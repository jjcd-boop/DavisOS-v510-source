#include "Kernel/NvmeAdminRuntime.hpp"
#include "Kernel/NvmeMmioTransport.hpp"
#include <cstdio>
using namespace Davis;
int main(){
 alignas(4096) unsigned char mmio[0x3000]{};
 alignas(64) Nvme::IoCommand sq[64]{};
 alignas(64) Nvme::IoCompletion cq[64]{};
 NvmeMmioTransport::State hw{};
 if(!NvmeMmioTransport::Bind(hw,mmio,sq,cq,64,4,0)) return 1;
 Nvme::QueueCreatePlan p{};
 if(!Nvme::BuildQueueCreatePlan(1,64,0x100000,0x200000,3,p)) return 2;
 cq[0].cid=11; cq[0].status=1; cq[1].cid=12; cq[1].status=1;
 NvmeAdminRuntime::Transport t{NvmeMmioTransport::Submit,NvmeMmioTransport::Poll,NvmeMmioTransport::Ring,NvmeMmioTransport::Reset,&hw};
 NvmeAdminRuntime::State st{};
 if(!NvmeAdminRuntime::CreateIoQueues(t,p,11,12,1000,st)||!st.ready) return 3;
 if(hw.submissions!=2||hw.completions!=2||hw.doorbellWrites!=4) return 4;
 // Admin commands MUST ring queue 0: SQ0 at 0x1000 and CQ0 at 0x1004.
 if(*(u32*)(mmio+0x1000)!=2||*(u32*)(mmio+0x1004)!=2) return 5;
 // I/O queue 1 doorbells must remain untouched while creating the queues via admin SQ/CQ.
 if(*(u32*)(mmio+0x1008)!=0||*(u32*)(mmio+0x100c)!=0) return 6;
 if(sq[0].opcode!=0x05||sq[1].opcode!=0x01||sq[0].cid!=11||sq[1].cid!=12) return 7;
 std::printf("PASS nvme_admin_doorbell admin_sq=%u admin_cq=%u doorbells=%llu io_db_untouched=1\n",*(u32*)(mmio+0x1000),*(u32*)(mmio+0x1004),(unsigned long long)hw.doorbellWrites);
}
