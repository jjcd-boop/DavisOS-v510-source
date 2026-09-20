#pragma once
#include "Nvme.hpp"
#include "NvmeAdminRuntime.hpp"
#include "NvmeMmioTransport.hpp"
#include "Dma.hpp"
namespace Davis::NvmeStorageRuntime {
enum class Stage:u8{Cold,Allocated,AdminBound,QueuesCreated,Ready,Failed};
struct State{Stage stage;Memory::DmaBuffer ioSq,ioCq;NvmeMmioTransport::State adminTransport,ioTransport;NvmeAdminRuntime::State adminRuntime;u16 depth,queueId;u64 bringups,failures;bool ready;};
bool BringUp(State&,Nvme::State&,Memory::PageAllocator&,u16 depth=(Nvme::MaxIoQueueDepth<64?Nvme::MaxIoQueueDepth:64),u16 queueId=1,u32 pollLimit=NvmeAdminRuntime::DefaultPollLimit);
void Shutdown(State&,Memory::PageAllocator&);
}
