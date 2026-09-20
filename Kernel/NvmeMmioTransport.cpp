#include "NvmeMmioTransport.hpp"
namespace Davis::NvmeMmioTransport {
static inline volatile u32* reg(State&s,u32 off){return (volatile u32*)(s.mmio+off);}
bool Bind(State&s,void*m,void*sq,void*cq,u16 depth,u32 stride,u16 qid){s={};if(!m||!sq||!cq||depth<2||depth>(Nvme::MaxAdminQueueDepth>Nvme::MaxIoQueueDepth?Nvme::MaxAdminQueueDepth:Nvme::MaxIoQueueDepth)||stride<4||(stride&(stride-1)))return false;if(((uptr)sq&3)||((uptr)cq&3))return false;u32 so=0,co=0;if(!Nvme::DoorbellOffsets(stride,qid,so,co))return false;s.mmio=(volatile u8*)m;s.sq=(Nvme::IoCommand*)sq;s.cq=(volatile Nvme::IoCompletion*)cq;s.depth=depth;s.doorbellStride=stride;s.queueId=qid;s.cqPhase=true;s.bound=true;return true;}

bool BindFromController(State&s,NvmeControllerTransport::State&c,void*sq,void*cq,u16 depth,u16 qid){
 if(!c.mmio||!c.capabilities.valid||c.stage!=NvmeControllerTransport::Stage::Ready||depth>c.capabilities.maxQueueEntries)return false;
 return Bind(s,(void*)c.mmio,sq,cq,depth,c.capabilities.doorbellStrideBytes,qid);
}
bool Submit(void*ctx,const Nvme::IoCommand&c){auto&s=*(State*)ctx;if(!s.bound||!c.cid||s.sqTail>=s.depth)return false;s.sq[s.sqTail]=c;__asm__ __volatile__("":::"memory");s.sqTail=(u16)((s.sqTail+1)%s.depth);s.submissions++;return true;}
bool Poll(void*ctx,u16 cid,u16&status){auto&s=*(State*)ctx;status=0xffff;if(!s.bound||!cid||s.cqHead>=s.depth)return false;volatile Nvme::IoCompletion&e=s.cq[s.cqHead];u16 st=e.status;if((st&1u)!=(s.cqPhase?1u:0u))return false;if(e.cid!=cid)return false;status=st;s.cqHead++;if(s.cqHead>=s.depth){s.cqHead=0;s.cqPhase=!s.cqPhase;}s.completions++;return true;}
bool PollAny(void*ctx,u16&cid,u16&status){auto&s=*(State*)ctx;cid=0;status=0xffff;if(!s.bound||s.cqHead>=s.depth)return false;volatile Nvme::IoCompletion&e=s.cq[s.cqHead];u16 st=e.status;if((st&1u)!=(s.cqPhase?1u:0u))return false;if(!e.cid)return false;cid=e.cid;status=st;s.cqHead++;if(s.cqHead>=s.depth){s.cqHead=0;s.cqPhase=!s.cqPhase;}s.completions++;return true;}
void Ring(void*ctx,bool submission,u16 qid){auto&s=*(State*)ctx;if(!s.bound||qid!=s.queueId)return;u32 so=0,co=0;if(!Nvme::DoorbellOffsets(s.doorbellStride,qid,so,co))return;*reg(s,submission?so:co)=submission?s.sqTail:s.cqHead;__asm__ __volatile__("mfence":::"memory");s.doorbellWrites++;}
void Reset(void*ctx){auto&s=*(State*)ctx;if(!s.bound)return;u32 cc=*reg(s,0x14);*reg(s,0x14)=cc&~1u;__asm__ __volatile__("mfence":::"memory");s.resets++;s.sqTail=0;s.cqHead=0;s.cqPhase=true;}
}
