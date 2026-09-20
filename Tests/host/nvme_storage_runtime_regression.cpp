#include "Kernel/NvmeStorageRuntime.hpp"
#include <atomic>
#include <thread>
#include <cstdio>
using namespace Davis;
alignas(4096) static unsigned char pool[4096*512];
alignas(4096) static unsigned char mmio[0x9000];
alignas(4096) static Nvme::IoCommand adminSq[64];
alignas(4096) static Nvme::IoCompletion adminCq[64];
int main(){
 Memory::PageAllocator a{};a.rangeCount=1;a.freeRanges[0]={ (u64)(uptr)pool,512};a.stats.freePages=512;a.stats.totalPages=512;
 Nvme::State c{};c.adminReady=true;c.identifyComplete=true;c.namespaceId=1;c.namespaceBlocks=1000000;c.logicalBlockBytes=512;c.mmio=(u64)(uptr)mmio;c.maxQueueEntries=64;c.doorbellStrideBytes=4;c.adminSq={ (u64)(uptr)adminSq,adminSq,sizeof(adminSq),1,4096};c.adminCq={ (u64)(uptr)adminCq,adminCq,sizeof(adminCq),1,4096};c.adminDepth=64;c.adminCqPhase=true;c.nextCommandId=1;
 std::atomic<bool> run{true};std::thread hw([&]{u32 done=0;while(run&&done<2){auto &q=adminSq[done];if(q.cid){adminCq[done].cid=q.cid;adminCq[done].status=1;done++;}else std::this_thread::yield();}});
 NvmeStorageRuntime::State s{};bool ok=NvmeStorageRuntime::BringUp(s,c,a,64,1,5000000);run=false;hw.join();if(!ok||!s.ready||s.adminRuntime.stage!=NvmeAdminRuntime::Stage::Ready)return 1;if(*(u32*)(mmio+0x1000)!=2||*(u32*)(mmio+0x1004)!=2)return 2;if(*(u32*)(mmio+0x1008)||*(u32*)(mmio+0x100c))return 3;u64 sq=s.ioSq.physical,cq=s.ioCq.physical;if(!sq||!cq)return 4;NvmeStorageRuntime::Shutdown(s,a);if(s.ioSq.physical||s.ioCq.physical)return 5;
 NvmeStorageRuntime::State bad{};c.maxQueueEntries=8;if(NvmeStorageRuntime::BringUp(bad,c,a,64,1,1000))return 6;
 std::printf("PASS nvme_storage_runtime admin_db=4 io_depth=64 dma_release=1 oversize_reject=1\n");
}
