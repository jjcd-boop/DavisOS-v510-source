#include "Kernel/ApLowMemory.hpp"
#include <cstdio>
using namespace Davis;
int main(){
 u32 bytes=ApLowMemory::TrampolineBytes(); if(bytes<128||bytes>ApLowMemory::HandoffOffset)return 1;
 u64 cycles=0;
 for(u32 n=0;n<100000;n++){
  ApTrampoline::Handoff h{}; if(!ApTrampoline::Build(h,n%256,n%64,0x1000,0xffff800000100000ull,0xffff800000200000ull,0xabc00000ull+n,n+1))return 2;
  ApLowMemory::Image i{};u64 p=0x1000ull+((n%255)*0x1000ull);if(!ApLowMemory::Build(i,p,h)||!ApLowMemory::Validate(i))return 3;
  if(i.bytes[0]!=0xfa||i.bytes[1]!=0xfc)return 4; // CLI, CLD entry contract
  cycles++;
 }
 ApTrampoline::Handoff h{};ApTrampoline::Build(h,1,1,0x1000,0x4000,0x5000,6,7);ApLowMemory::Image i{};ApLowMemory::Build(i,0x8000,h);i.bytes[10]^=0x80;if(ApLowMemory::Validate(i))return 5;
 std::printf("PASS ap_executable_trampoline cycles=%llu code_bytes=%u handoff_offset=%u corrupt_code_reject=1\n",(unsigned long long)cycles,bytes,ApLowMemory::HandoffOffset);
}
