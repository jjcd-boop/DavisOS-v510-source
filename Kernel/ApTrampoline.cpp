#include "ApTrampoline.hpp"
namespace Davis::ApTrampoline {
static u64 mix(u64 x){x^=x>>30;x*=0xbf58476d1ce4e5b9ull;x^=x>>27;x*=0x94d049bb133111ebull;x^=x>>31;return x;}
static u32 rol5(u32 x){return (x<<5)|(x>>27);}
u32 ComputePrePagingSeal(const Handoff&h){
 // Assembly-friendly integrity seal over every field consumed before the C++ validator.
 // The first 64 bytes are sixteen little-endian dwords ending at generation.
 const u8*b=(const u8*)&h;u32 s=0xD415A55Au;
 for(u32 i=0;i<16;i++){u32 w=(u32)b[i*4]|((u32)b[i*4+1]<<8)|((u32)b[i*4+2]<<16)|((u32)b[i*4+3]<<24);s^=w;s=rol5(s);s+=0x9e3779b9u;}
 return s;
}
u64 ComputeChecksum(const Handoff&h){return mix(h.magic)^mix(((u64)h.version<<32)|h.bytes)^mix(((u64)h.apicId<<32)|h.logicalId)^mix(h.cr3)^mix(h.stackTop)^mix(h.entryPoint)^mix(h.rendezvousToken)^mix(h.generation)^mix(h.prePagingSeal);}
bool Build(Handoff&h,u32 apic,u32 logical,u64 cr3,u64 stack,u64 entry,u64 token,u64 gen){h={};if(!cr3||!stack||!entry||!token||!gen||(cr3&0xfffull)||(stack&0xfull))return false;h.magic=HandoffMagic;h.version=HandoffVersion;h.bytes=sizeof(Handoff);h.apicId=apic;h.logicalId=logical;h.cr3=cr3;h.stackTop=stack;h.entryPoint=entry;h.rendezvousToken=token;h.generation=gen;h.prePagingSeal=ComputePrePagingSeal(h);h.checksum=ComputeChecksum(h);return true;}
bool ValidatePrePaging(const Handoff&h){return h.magic==HandoffMagic&&h.version==HandoffVersion&&h.bytes==sizeof(Handoff)&&h.cr3&&h.cr3<=0xffffffffull&&(h.cr3&0xfffull)==0&&h.stackTop&&(h.stackTop&0xfull)==0&&h.entryPoint&&h.rendezvousToken&&h.generation&&h.prePagingSeal==ComputePrePagingSeal(h);}
bool Validate(const Handoff&h){if(!ValidatePrePaging(h))return false;return h.checksum==ComputeChecksum(h);}
}
