#pragma once
#include "../Base/Types.hpp"
#include "Pci.hpp"
#include "Dma.hpp"
namespace Davis::Nvme {
constexpr u16 MaxAdminQueueDepth=64;
constexpr u16 MaxIoQueueDepth=128;
enum class Result:u32 { Cold=0,NotFound,InvalidBar,CapabilitiesReady,UnsupportedPageSize,DmaUnavailable,DisableTimeout,EnableTimeout,ControllerFatal,AdminQueuesReady,IdentifyDmaUnavailable,AdminCommandTimeout,AdminCommandFailed,ControllerIdentified,NamespaceListIdentified,NamespaceIdentified };
enum class Phase:u8 { Cold=0,Discovered,Capabilities,Disabling,QueuesAllocated,Enabling,Ready,IdentifyingController,IdentifyingNamespaces,NamespaceReady,Failed };
enum class IoOpcode:u8 { Write=0x01, Read=0x02, Flush=0x00 };
struct IoCommand { u8 opcode,flags; u16 cid; u32 nsid; u64 rsvd2; u64 mptr; u64 prp1,prp2; u32 cdw10,cdw11,cdw12,cdw13,cdw14,cdw15; };
static_assert(sizeof(IoCommand)==64,"NVMe I/O SQE size");
enum class IoSlotState:u8 { Free=0,Submitted=1,Completed=2,Failed=3 };
struct IoSlot { u16 cid; IoSlotState state; IoOpcode opcode; u64 lba; u16 blocks; u16 status; };
struct IoQueueState { u16 depth; u16 outstanding; u16 nextCid; u64 submitted; u64 completed; u64 failed; IoSlot slots[MaxIoQueueDepth]; };
#pragma pack(push,1)
struct IoCompletion { u32 result,rsvd; u16 sqHead,sqId,cid,status; };
#pragma pack(pop)
static_assert(sizeof(IoCompletion)==16,"NVMe I/O CQE size");
struct IoRing { IoCommand* sq; volatile IoCompletion* cq; u16 depth,sqTail,cqHead; bool cqPhase; u64 sqDoorbells,cqDoorbells; };
struct QueueCreatePlan { IoCommand createCq; IoCommand createSq; u16 queueId,depth; u64 sqPhysical,cqPhysical; bool valid; };
bool BuildQueueCreatePlan(u16 queueId,u16 depth,u64 sqPhysical,u64 cqPhysical,u16 interruptVector,QueueCreatePlan&);
constexpr u16 MaxPrpListEntries=512;
struct PrpPlan { u64 prp1,prp2; u16 listEntries; u64 bytes; bool usesList; };
bool BuildPrpPlan(u64 firstPhysical,u64 bytes,u64 pageBytes,u64* listMemory,u16 listCapacity,PrpPlan&);
bool DoorbellOffsets(u32 doorbellStrideBytes,u16 queueId,u32& sqOffset,u32& cqOffset);
struct State {
 bool discovered,barValid,capabilitiesValid,controllerEnabled,adminReady;
 u8 bus,device,function; u16 vendorId,deviceId; u64 mmio,cap; u32 version,doorbellStrideBytes; u16 maxQueueEntries; u8 timeoutUnits500ms,minPageShift,maxPageShift;
 u16 adminDepth,adminSqTail,adminCqHead,nextCommandId; bool adminCqPhase;
 Memory::DmaBuffer adminSq,adminCq,identify; u32 namespaceId; u64 namespaceBlocks; u32 logicalBlockBytes; u32 namespaceCount; bool identifyComplete; Phase phase; Result result; u64 initPolls;
};
void Init(State&); bool Discover(State&,const Pci::State&); bool ReadCapabilities(State&); bool ValidateHostPageSize(State&,u32 pageBytes=4096);
bool BeginAdminController(State&,Memory::PageAllocator&); bool IdentifyStorage(State&,Memory::PageAllocator&);
bool BuildIoCommand(const State&,IoOpcode,u16 cid,u64 lba,u16 blockCount,u64 prp1,u64 prp2,IoCommand&);
bool CompletionSucceeded(u16 status,u16 expectedCid,u16 actualCid);
bool InitIoQueue(IoQueueState&,u16 depth);
bool SubmitIo(IoQueueState&,const State&,IoOpcode,u64 lba,u16 blocks,u64 prp1,u64 prp2,IoCommand&,u16& cid);
bool CompleteIo(IoQueueState&,u16 cid,u16 status);
bool ReapIo(IoQueueState&,u16 cid,bool& success);
bool CancelIo(IoQueueState&,u16 cid);
bool BindIoRing(IoRing&,void* sqMemory,void* cqMemory,u16 depth);
bool PublishIo(IoRing&,const IoCommand&);
bool ConsumeIoCompletion(IoRing&,IoQueueState&,u16& cid,bool& success);
void Shutdown(State&,Memory::PageAllocator&);
}
