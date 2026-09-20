#pragma once
#include "Nvme.hpp"
#include "NvmeMmioTransport.hpp"
#include "NvmeStorageRuntime.hpp"
#include "BlockDevice.hpp"
#include "Dma.hpp"
namespace Davis::NvmeBlockBackend {
using MapBuffer = bool(*)(void* virtualAddress,u64 bytes,u64& physicalAddress);
constexpr u64 MaxBounceBytes=256ull*1024ull;
struct Transfer { u16 cid; BlockDevice::Op op; void* user; u64 bytes; Memory::DmaBuffer bounce,prpList; bool active; };
struct Binding {
 Nvme::State* controller; Nvme::IoQueueState queue; Nvme::IoRing ring; NvmeMmioTransport::State* mmioTransport;
 Memory::PageAllocator* allocator; MapBuffer mapper; Transfer transfers[Nvme::MaxIoQueueDepth];
 u32 deviceId,backendId; u64 submitted,completed,failed,bounced; bool ready,useMmio;
};
extern Binding binding;
bool Bind(Nvme::State&,void* sqMemory,void* cqMemory,u16 depth,MapBuffer,u32 backendId=2,u32 deviceId=2);
bool BindRuntime(Nvme::State&,NvmeStorageRuntime::State&,Memory::PageAllocator&,u32 backendId=2,u32 deviceId=2);
void Unbind();
}
