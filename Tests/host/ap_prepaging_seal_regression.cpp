#include "Kernel/ApTrampoline.hpp"
#include <cstdio>
using namespace Davis;
int main(){
 unsigned long long cycles=0,rejects=0;
 for(unsigned i=0;i<100000;i++){
  ApTrampoline::Handoff h{};
  u64 stack=0xffff800000200000ull+((u64)(i&255)<<12);
  u64 entry=0xffffffff80001000ull+((u64)(i&63)<<4);
  if(!ApTrampoline::Build(h,1+(i&31),1+(i&31),0x2000+(u64)(i&255)*0x1000,stack,entry,0x1122334455667788ull+i,1+i))return 1;
  if(!ApTrampoline::ValidatePrePaging(h)||!ApTrampoline::Validate(h))return 2;
  auto corrupt=[&](u32 off){auto c=h;((u8*)&c)[off]^=0x40;if(ApTrampoline::ValidatePrePaging(c))return false;rejects++;return true;};
  if(!corrupt(ApTrampoline::HandoffOffsetStackTop)||!corrupt(ApTrampoline::HandoffOffsetEntryPoint)||!corrupt(ApTrampoline::HandoffOffsetToken)||!corrupt(ApTrampoline::HandoffOffsetGeneration))return 3;
  auto seal=h;seal.prePagingSeal^=1;if(ApTrampoline::ValidatePrePaging(seal))return 4;rejects++;
  cycles++;
 }
 std::printf("PASS ap_prepaging_seal cycles=%llu corruption_rejects=%llu abi=%u bytes=%zu\n",cycles,rejects,ApTrampoline::HandoffVersion,sizeof(ApTrampoline::Handoff));
 return 0;
}
