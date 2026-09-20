#pragma once
#include "../Base/Types.hpp"
#include "Nvme.hpp"
#include "NvmeControllerTransport.hpp"
namespace Davis::NvmeMmioTransport {
struct State {
 volatile u8* mmio; Nvme::IoCommand* sq; volatile Nvme::IoCompletion* cq;
 u16 depth,sqTail,cqHead; bool cqPhase; u32 doorbellStride; u16 queueId;
 u64 submissions,completions,doorbellWrites,resets; bool bound;
};
bool Bind(State&,void* mmio,void* sq,void* cq,u16 depth,u32 doorbellStride,u16 queueId=0);
bool BindFromController(State&,NvmeControllerTransport::State&,void* sq,void* cq,u16 depth,u16 queueId=0);
bool Submit(void*,const Nvme::IoCommand&);
bool Poll(void*,u16 cid,u16& status);
bool PollAny(void*,u16& cid,u16& status);
void Ring(void*,bool submission,u16 queueId);
void Reset(void*);
}
