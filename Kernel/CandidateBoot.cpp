#include "CandidateBoot.hpp"
namespace Davis::CandidateBoot {
struct Guid{u32 a;u16 b,c;u8 d[8];};
struct Hdr{u64 sig;u32 rev,size,crc,res;};
struct RuntimeServices{Hdr hdr;void*getTime;void*setTime;void*getWakeupTime;void*setWakeupTime;void*setVirtualAddressMap;void*convertPointer;void*getVariable;void*getNextVariableName;u64(__attribute__((ms_abi))*setVariable)(u16*,Guid*,u32,usize,void*);};
#pragma pack(push,1)
struct State{u32 magic,stageDigest,state,checkpoint;};
#pragma pack(pop)
static constexpr u32 Magic=0x54424344;
static Guid guid={0x6f8d7a31,0x9b5e,0x4c31,{0xa4,0x73,0x44,0x41,0x56,0x49,0x53,0x23}};
static u16 name[]={'D','a','v','i','s','C','a','n','d','i','d','a','t','e','B','o','o','t',0};
static void Write(const BootInfo&b,u32 state,u32 checkpoint){
 if(!b.candidateBoot||!b.candidateStageDigest||!b.runtimeServices)return;
 auto*rt=(RuntimeServices*)b.runtimeServices;if(!rt->setVariable)return;
 State s{Magic,b.candidateStageDigest,state,checkpoint};constexpr u32 attrs=1|2|4;
 rt->setVariable(name,&guid,attrs,sizeof(s),&s);
}
void Mark(const BootInfo&b,Checkpoint c){Write(b,1,(u32)c);}
void AcknowledgeHealthy(const BootInfo&b){Write(b,2,(u32)Checkpoint::Healthy);}
}
