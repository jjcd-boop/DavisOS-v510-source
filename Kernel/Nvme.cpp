#include "Nvme.hpp"
namespace Davis::Nvme {
static inline volatile u32* r32(State&s,u32 o){return (volatile u32*)((volatile u8*)(uptr)s.mmio+o);} static inline volatile u64* r64(State&s,u32 o){return (volatile u64*)((volatile u8*)(uptr)s.mmio+o);}
static bool waitReady(State&s,bool want){u64 limit=(u64)(s.timeoutUnits500ms?s.timeoutUnits500ms:1u)*12000000ull;for(u64 i=0;i<limit;i++){u32 c=*r32(s,0x1c);s.initPolls++;if(c&(1u<<1)){s.result=Result::ControllerFatal;s.phase=Phase::Failed;return false;}if(((c&1u)!=0)==want)return true;}return false;}
void Init(State&s){s={};s.phase=Phase::Cold;s.result=Result::Cold;}
bool Discover(State&s,const Pci::State&p){Init(s);if(!p.nvmeFound){s.result=Result::NotFound;return false;}const auto&d=p.nvme;s.discovered=true;s.bus=d.bus;s.device=d.slot;s.function=d.function;s.vendorId=d.vendor;s.deviceId=d.device;u32 lo=Pci::ReadConfig(d.bus,d.slot,d.function,0x10);if(lo&1u){s.result=Result::InvalidBar;return false;}u32 type=(lo>>1)&3u;if(type==1u||type==3u){s.result=Result::InvalidBar;return false;}u64 base=(u64)(lo&~0xFull);if(type==2u)base|=((u64)Pci::ReadConfig(d.bus,d.slot,d.function,0x14)<<32);if(!base||(base&0xFFFu)){s.result=Result::InvalidBar;return false;}s.mmio=base;s.barValid=true;s.phase=Phase::Discovered;return true;}
bool ReadCapabilities(State&s){if(!s.discovered||!s.barValid)return false;s.cap=*r64(s,0x00);s.version=*r32(s,0x08);s.maxQueueEntries=(u16)((s.cap&0xFFFFu)+1u);s.timeoutUnits500ms=(u8)((s.cap>>24)&0xFFu);s.doorbellStrideBytes=4u<<(u32)((s.cap>>32)&0xFu);s.minPageShift=(u8)(12u+((s.cap>>48)&0xFu));s.maxPageShift=(u8)(12u+((s.cap>>52)&0xFu));if(!s.maxQueueEntries||s.minPageShift>s.maxPageShift){s.result=Result::InvalidBar;s.phase=Phase::Failed;return false;}s.capabilitiesValid=true;s.result=Result::CapabilitiesReady;s.phase=Phase::Capabilities;return true;}
bool ValidateHostPageSize(State&s,u32 pageBytes){if(!s.capabilitiesValid||!pageBytes||(pageBytes&(pageBytes-1)))return false;u32 shift=0,v=pageBytes;while(v>1){v>>=1;shift++;}if(shift<s.minPageShift||shift>s.maxPageShift){s.result=Result::UnsupportedPageSize;s.phase=Phase::Failed;return false;}return true;}
bool BeginAdminController(State&s,Memory::PageAllocator&a){if(!ValidateHostPageSize(s,4096))return false;s.phase=Phase::Disabling;u32 cc=*r32(s,0x14);if(cc&1u){*r32(s,0x14)=cc&~1u;if(!waitReady(s,false)){s.result=Result::DisableTimeout;s.phase=Phase::Failed;return false;}}else if((*r32(s,0x1c)&1u)!=0){if(!waitReady(s,false)){s.result=Result::DisableTimeout;s.phase=Phase::Failed;return false;}}
 u16 depth=s.maxQueueEntries<MaxAdminQueueDepth?s.maxQueueEntries:MaxAdminQueueDepth;if(depth<2){s.result=Result::InvalidBar;s.phase=Phase::Failed;return false;}s.adminSq=Memory::AllocDma(a,(u64)depth*64,4096);s.adminCq=Memory::AllocDma(a,(u64)depth*16,4096);if(!s.adminSq.physical||!s.adminCq.physical){if(s.adminSq.physical)Memory::FreeDma(a,s.adminSq);if(s.adminCq.physical)Memory::FreeDma(a,s.adminCq);s.result=Result::DmaUnavailable;s.phase=Phase::Failed;return false;}s.adminDepth=depth;s.adminCqPhase=true;s.phase=Phase::QueuesAllocated;
 *r32(s,0x24)=((u32)(depth-1)<<16)|(u32)(depth-1);*r64(s,0x28)=s.adminSq.physical;*r64(s,0x30)=s.adminCq.physical;
 u32 mps=(12u-s.minPageShift);(void)mps; // 4KiB host page validated; CC.MPS is relative to 4KiB, therefore zero.
 u32 newcc=(6u<<16)|(4u<<20)|1u;*r32(s,0x14)=newcc;s.phase=Phase::Enabling;if(!waitReady(s,true)){*r32(s,0x14)=0;waitReady(s,false);Memory::FreeDma(a,s.adminSq);Memory::FreeDma(a,s.adminCq);s.result=Result::EnableTimeout;s.phase=Phase::Failed;return false;}s.controllerEnabled=true;s.adminReady=true;s.adminSqTail=0;s.adminCqHead=0;s.nextCommandId=1;s.result=Result::AdminQueuesReady;s.phase=Phase::Ready;return true;}

#pragma pack(push,1)
struct AdminCommand { u8 opcode,flags; u16 cid; u32 nsid; u64 rsvd2; u64 mptr; u64 prp1,prp2; u32 cdw10,cdw11,cdw12,cdw13,cdw14,cdw15; };
struct Completion { u32 result,rsvd; u16 sqHead,sqId,cid,status; };
#pragma pack(pop)
static_assert(sizeof(AdminCommand)==64,"NVMe SQE size"); static_assert(sizeof(Completion)==16,"NVMe CQE size");
static inline void ringSq(State&s){*r32(s,0x1000)=s.adminSqTail;}
static inline void ringCq(State&s){*r32(s,0x1000+s.doorbellStrideBytes)=s.adminCqHead;}
static bool submitIdentify(State&s,u32 nsid,u32 cns,u64 prp,u32&result){
 if(!s.adminReady||!s.adminSq.virtualAddress||!s.adminCq.virtualAddress)return false;
 auto*sq=(volatile AdminCommand*)s.adminSq.virtualAddress; auto*cq=(volatile Completion*)s.adminCq.virtualAddress;
 u16 cid=s.nextCommandId++; if(!s.nextCommandId)s.nextCommandId=1; u16 slot=s.adminSqTail;
 volatile AdminCommand&c=sq[slot]; volatile u8*cz=(volatile u8*)&c; for(u32 z=0;z<64;z++)cz[z]=0; c.opcode=0x06; c.cid=cid; c.nsid=nsid; c.prp1=prp; c.cdw10=cns;
 s.adminSqTail=(u16)((s.adminSqTail+1)%s.adminDepth); ringSq(s);
 u64 limit=(u64)(s.timeoutUnits500ms?s.timeoutUnits500ms:1u)*12000000ull;
 for(u64 i=0;i<limit;i++){
  volatile Completion&e=cq[s.adminCqHead]; u16 st=e.status;
  if((st&1u)!=(s.adminCqPhase?1u:0u))continue;
  if(e.cid!=cid){s.result=Result::AdminCommandFailed;s.phase=Phase::Failed;return false;}
  result=e.result; u16 code=(u16)(st>>1); s.adminCqHead++; if(s.adminCqHead>=s.adminDepth){s.adminCqHead=0;s.adminCqPhase=!s.adminCqPhase;} ringCq(s);
  if(code){s.result=Result::AdminCommandFailed;s.phase=Phase::Failed;return false;} return true;
 }
 s.result=Result::AdminCommandTimeout;s.phase=Phase::Failed;return false;
}
static u32 rd32(const u8*p,u32 o){return (u32)p[o]|((u32)p[o+1]<<8)|((u32)p[o+2]<<16)|((u32)p[o+3]<<24);}
static u64 rd64(const u8*p,u32 o){return (u64)rd32(p,o)|((u64)rd32(p,o+4)<<32);}
bool IdentifyStorage(State&s,Memory::PageAllocator&a){
 if(!s.adminReady)return false; if(!s.identify.physical)s.identify=Memory::AllocDma(a,4096,4096); if(!s.identify.physical){s.result=Result::IdentifyDmaUnavailable;s.phase=Phase::Failed;return false;}
 u32 result=0; Memory::Zero(s.identify); s.phase=Phase::IdentifyingController;
 if(!submitIdentify(s,0,1,s.identify.physical,result))return false; auto*b=(u8*)s.identify.virtualAddress; s.namespaceCount=rd32(b,516); s.result=Result::ControllerIdentified;
 Memory::Zero(s.identify); s.phase=Phase::IdentifyingNamespaces; if(!submitIdentify(s,0,2,s.identify.physical,result))return false; s.namespaceId=rd32((u8*)s.identify.virtualAddress,0); if(!s.namespaceId){s.result=Result::AdminCommandFailed;s.phase=Phase::Failed;return false;} s.result=Result::NamespaceListIdentified;
 Memory::Zero(s.identify); if(!submitIdentify(s,s.namespaceId,0,s.identify.physical,result))return false; b=(u8*)s.identify.virtualAddress; s.namespaceBlocks=rd64(b,0); u8 flbas=b[26]; u8 fmt=flbas&0x0Fu; u32 lbaf=rd32(b,128u+(u32)fmt*4u); u8 ds=(u8)((lbaf>>16)&0xFFu); if(!s.namespaceBlocks||ds<9||ds>16){s.result=Result::AdminCommandFailed;s.phase=Phase::Failed;return false;} s.logicalBlockBytes=1u<<ds; s.identifyComplete=true;s.result=Result::NamespaceIdentified;s.phase=Phase::NamespaceReady;return true;
}

bool BuildIoCommand(const State&s,IoOpcode op,u16 cid,u64 lba,u16 blocks,u64 prp1,u64 prp2,IoCommand&out){
 out={};if(!s.identifyComplete||!s.namespaceId||!s.namespaceBlocks||!s.logicalBlockBytes||!cid)return false;
 if(op!=IoOpcode::Read&&op!=IoOpcode::Write&&op!=IoOpcode::Flush)return false;
 if(op==IoOpcode::Flush){out.opcode=(u8)op;out.cid=cid;out.nsid=s.namespaceId;return true;}
 if(!blocks||!prp1||(prp1&3u))return false;if(lba>=s.namespaceBlocks||(u64)blocks>s.namespaceBlocks-lba)return false;
 u64 bytes=(u64)blocks*s.logicalBlockBytes;u64 firstPage=4096u-(prp1&4095u);if(bytes>firstPage&&!prp2)return false;
 out.opcode=(u8)op;out.cid=cid;out.nsid=s.namespaceId;out.prp1=prp1;out.prp2=prp2;out.cdw10=(u32)lba;out.cdw11=(u32)(lba>>32);out.cdw12=(u32)(blocks-1);return true;
}
bool CompletionSucceeded(u16 status,u16 expectedCid,u16 actualCid){if(!expectedCid||expectedCid!=actualCid)return false;return ((status>>1)&0x7ffu)==0;}

bool InitIoQueue(IoQueueState&q,u16 depth){
 q={};if(depth<2||depth>MaxIoQueueDepth)return false;q.depth=depth;q.nextCid=1;return true;
}
static IoSlot* findSlot(IoQueueState&q,u16 cid){if(!cid)return nullptr;for(u16 i=0;i<q.depth;i++)if(q.slots[i].state!=IoSlotState::Free&&q.slots[i].cid==cid)return &q.slots[i];return nullptr;}
bool SubmitIo(IoQueueState&q,const State&s,IoOpcode op,u64 lba,u16 blocks,u64 prp1,u64 prp2,IoCommand&cmd,u16&cid){
 cid=0;if(q.depth<2||q.depth>MaxIoQueueDepth||q.outstanding>=q.depth-1)return false;
 u16 slot=q.depth;for(u16 i=0;i<q.depth;i++)if(q.slots[i].state==IoSlotState::Free){slot=i;break;}if(slot==q.depth)return false;
 u16 candidate=q.nextCid;for(u32 tries=0;tries<65535;tries++){if(!candidate)candidate=1;if(!findSlot(q,candidate))break;candidate++;}
 if(!candidate||findSlot(q,candidate))return false;
 if(!BuildIoCommand(s,op,candidate,lba,blocks,prp1,prp2,cmd))return false;
 auto&x=q.slots[slot];x={};x.cid=candidate;x.state=IoSlotState::Submitted;x.opcode=op;x.lba=lba;x.blocks=blocks;q.outstanding++;q.submitted++;q.nextCid=(u16)(candidate+1);if(!q.nextCid)q.nextCid=1;cid=candidate;return true;
}
bool CompleteIo(IoQueueState&q,u16 cid,u16 status){auto*x=findSlot(q,cid);if(!x||x->state!=IoSlotState::Submitted)return false;x->status=status;if(CompletionSucceeded(status,cid,cid)){x->state=IoSlotState::Completed;q.completed++;}else{x->state=IoSlotState::Failed;q.failed++;}return true;}
bool ReapIo(IoQueueState&q,u16 cid,bool&success){auto*x=findSlot(q,cid);if(!x||(x->state!=IoSlotState::Completed&&x->state!=IoSlotState::Failed))return false;success=x->state==IoSlotState::Completed;*x={};if(q.outstanding)q.outstanding--;return true;}
bool CancelIo(IoQueueState&q,u16 cid){auto*x=findSlot(q,cid);if(!x||x->state!=IoSlotState::Submitted)return false;*x={};if(q.outstanding)q.outstanding--;q.failed++;return true;}

bool BuildPrpPlan(u64 firstPhysical,u64 bytes,u64 pageBytes,u64* listMemory,u16 listCapacity,PrpPlan&out){
 out={};if(!firstPhysical||!bytes||pageBytes<4096||(pageBytes&(pageBytes-1)))return false;
 u64 firstRemain=pageBytes-(firstPhysical&(pageBytes-1));out.prp1=firstPhysical;out.bytes=bytes;
 if(bytes<=firstRemain)return true;
 u64 remain=bytes-firstRemain;u64 next=(firstPhysical+firstRemain);
 if(next&(pageBytes-1))return false;
 if(remain<=pageBytes){out.prp2=next;return true;}
 u64 pages=(remain+pageBytes-1)/pageBytes;if(!listMemory||pages>listCapacity||pages>MaxPrpListEntries)return false;
 for(u64 i=0;i<pages;i++)listMemory[i]=next+i*pageBytes;
 out.prp2=(u64)(uptr)listMemory;out.listEntries=(u16)pages;out.usesList=true;return true;
}
bool DoorbellOffsets(u32 stride,u16 queueId,u32&sqOffset,u32&cqOffset){
 sqOffset=cqOffset=0;if(!stride||stride<4||(stride&(stride-1)))return false;
 u64 sq=0x1000ull+(u64)(2u*queueId)*stride;u64 cq=sq+stride;if(sq>0xffffffffull||cq>0xffffffffull)return false;sqOffset=(u32)sq;cqOffset=(u32)cq;return true;
}

bool BindIoRing(IoRing&r,void*sqMemory,void*cqMemory,u16 depth){
 r={};if(!sqMemory||!cqMemory||depth<2||depth>MaxIoQueueDepth)return false;
 if(((uptr)sqMemory&3u)||((uptr)cqMemory&3u))return false;
 r.sq=(IoCommand*)sqMemory;r.cq=(volatile IoCompletion*)cqMemory;r.depth=depth;r.cqPhase=true;return true;
}
bool PublishIo(IoRing&r,const IoCommand&cmd){
 if(!r.sq||!r.cq||r.depth<2||r.sqTail>=r.depth||!cmd.cid)return false;
 r.sq[r.sqTail]=cmd;r.sqTail=(u16)((r.sqTail+1u)%r.depth);r.sqDoorbells++;return true;
}
bool ConsumeIoCompletion(IoRing&r,IoQueueState&q,u16&cid,bool&success){
 cid=0;success=false;if(!r.cq||!r.sq||r.depth<2||r.cqHead>=r.depth)return false;
 volatile IoCompletion&e=r.cq[r.cqHead];u16 status=e.status;
 if((status&1u)!=(r.cqPhase?1u:0u))return false;
 u16 got=e.cid;if(!got||!CompleteIo(q,got,status))return false;
 cid=got;if(!ReapIo(q,got,success))return false;
 r.cqHead++;if(r.cqHead>=r.depth){r.cqHead=0;r.cqPhase=!r.cqPhase;}r.cqDoorbells++;return true;
}

void Shutdown(State&s,Memory::PageAllocator&a){if(s.barValid&&s.controllerEnabled){*r32(s,0x14)=*r32(s,0x14)&~1u;waitReady(s,false);}s.controllerEnabled=false;s.adminReady=false;if(s.adminSq.physical)Memory::FreeDma(a,s.adminSq);if(s.adminCq.physical)Memory::FreeDma(a,s.adminCq);if(s.identify.physical)Memory::FreeDma(a,s.identify);}
}

namespace Davis::Nvme {
bool BuildQueueCreatePlan(u16 qid,u16 depth,u64 sqp,u64 cqp,u16 iv,QueueCreatePlan&p){
 p={};if(!qid||depth<2||depth>MaxIoQueueDepth||!sqp||!cqp||(sqp&0xfffull)||(cqp&0xfffull))return false;
 // Admin Create I/O Completion Queue (0x05), physically contiguous + interrupts enabled.
 p.createCq={};p.createCq.opcode=0x05;p.createCq.prp1=cqp;p.createCq.cdw10=(u32)qid|((u32)(depth-1)<<16);p.createCq.cdw11=1u|2u|((u32)iv<<16);
 // Admin Create I/O Submission Queue (0x01), physically contiguous, bound to CQ qid.
 p.createSq={};p.createSq.opcode=0x01;p.createSq.prp1=sqp;p.createSq.cdw10=(u32)qid|((u32)(depth-1)<<16);p.createSq.cdw11=1u|((u32)qid<<16);
 p.queueId=qid;p.depth=depth;p.sqPhysical=sqp;p.cqPhysical=cqp;p.valid=true;return true;
}
}
