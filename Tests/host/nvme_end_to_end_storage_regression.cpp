#include "Kernel/NvmeStorageRuntime.hpp"
#include "Kernel/NvmeBlockBackend.hpp"
#include <atomic>
#include <thread>
#include <cstdio>
#include <cstring>
using namespace Davis;
alignas(4096) static unsigned char pool[4096*2048];
alignas(4096) static unsigned char mmio[0x9000];
alignas(4096) static Nvme::IoCommand adminSq[64];
alignas(4096) static Nvme::IoCompletion adminCq[64];
static unsigned char disk[512*4096];
int main(){
 Memory::PageAllocator a{};a.rangeCount=1;a.freeRanges[0]={(u64)(uptr)pool,2048};a.stats.freePages=2048;a.stats.totalPages=2048;
 Nvme::State c{};c.adminReady=true;c.identifyComplete=true;c.namespaceId=1;c.namespaceBlocks=4096;c.logicalBlockBytes=512;c.mmio=(u64)(uptr)mmio;c.maxQueueEntries=64;c.doorbellStrideBytes=4;c.adminSq={(u64)(uptr)adminSq,adminSq,sizeof(adminSq),1,4096};c.adminCq={(u64)(uptr)adminCq,adminCq,sizeof(adminCq),1,4096};c.adminDepth=64;c.adminCqPhase=true;c.nextCommandId=1;
 std::atomic<bool> arun{true};std::thread admin([&]{u32 done=0;while(arun&&done<2){auto&q=adminSq[done];if(q.cid){adminCq[done].cid=q.cid;adminCq[done].status=1;done++;}else std::this_thread::yield();}});
 NvmeStorageRuntime::State rt{};if(!NvmeStorageRuntime::BringUp(rt,c,a,64,1,5000000))return 1;arun=false;admin.join();
 BlockDevice::Init();if(!NvmeBlockBackend::BindRuntime(c,rt,a,2,2))return 2;
 auto*sq=(Nvme::IoCommand*)rt.ioSq.virtualAddress;auto*cq=(Nvme::IoCompletion*)rt.ioCq.virtualAddress;std::atomic<bool> run{true};std::thread hw([&]{u16 sh=0,ch=0;bool phase=true;while(run){u32 tail=*(volatile u32*)(mmio+0x1008);while(sh!=tail){auto cmd=sq[sh];u16 blocks=cmd.opcode==0?0:(u16)((cmd.cdw12&0xffff)+1);u64 lba=(u64)cmd.cdw10|((u64)cmd.cdw11<<32);u64 bytes=(u64)blocks*512;if(cmd.opcode==1)std::memcpy(disk+lba*512,(void*)(uptr)cmd.prp1,bytes);else if(cmd.opcode==2)std::memcpy((void*)(uptr)cmd.prp1,disk+lba*512,bytes);cq[ch]={};cq[ch].cid=cmd.cid;cq[ch].status=phase?1:0;sh=(u16)((sh+1)%64);ch++;if(ch==64){ch=0;phase=!phase;}}std::this_thread::yield();}});
 alignas(64) unsigned char w[32768],r[32768];for(u32 i=0;i<sizeof(w);i++)w[i]=(u8)(i*37u+11u);std::memset(r,0,sizeof(r));
 auto wait=[&](u64 id){for(u32 n=0;n<1000000;n++){BlockDevice::PumpOnce();auto st=BlockDevice::Query(id);if(st==BlockDevice::Status::Complete||st==BlockDevice::Status::Failed||st==BlockDevice::Status::TimedOut)return st;if((n&255u)==0)std::this_thread::yield();}return BlockDevice::Status::TimedOut;};
 int failure=0;for(u32 i=0;i<2000&&!failure;i++){u32 sectors=1+(i%64);u64 lba=(i*67)%3000;u64 wid=BlockDevice::Submit(2,BlockDevice::Op::Write,lba,sectors,w,100000);if(!wid||wait(wid)!=BlockDevice::Status::Complete||!BlockDevice::Reap(wid)){failure=3;break;}std::memset(r,0,sectors*512);u64 rid=BlockDevice::Submit(2,BlockDevice::Op::Read,lba,sectors,r,100000);if(!rid||wait(rid)!=BlockDevice::Status::Complete||!BlockDevice::Reap(rid)){failure=4;break;}if(std::memcmp(w,r,sectors*512)){failure=5;break;}if((i%97)==0){u64 fid=BlockDevice::Submit(2,BlockDevice::Op::Flush,0,0,nullptr,100000);if(!fid||wait(fid)!=BlockDevice::Status::Complete||!BlockDevice::Reap(fid)){failure=6;break;}}}
 run=false;hw.join();if(failure)return failure;if(NvmeBlockBackend::binding.queue.outstanding)return 7;if(NvmeBlockBackend::binding.bounced<4000)return 8;
 std::printf("PASS nvme_end_to_end cycles=2000 io=%llu bounced=%llu sqdb=%llu cqdb=%llu flush=1\n",(unsigned long long)NvmeBlockBackend::binding.submitted,(unsigned long long)NvmeBlockBackend::binding.bounced,(unsigned long long)rt.ioTransport.doorbellWrites/2,(unsigned long long)rt.ioTransport.doorbellWrites/2);
 NvmeBlockBackend::Unbind();NvmeStorageRuntime::Shutdown(rt,a);return 0;
}
