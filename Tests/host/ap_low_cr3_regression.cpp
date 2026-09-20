#include "Kernel/ApLowMemory.hpp"
#include <cstdio>
using namespace Davis;
int main(){ApTrampoline::Handoff h{};ApLowMemory::Image i{};if(!ApTrampoline::Build(h,1,1,0xfffff000ull,0x200000,0xffffffff80001000ull,7,1))return 1;if(!ApLowMemory::Build(i,0x8000,h))return 2;if(!ApTrampoline::Build(h,1,1,0x100000000ull,0x200000,0xffffffff80001000ull,7,1))return 3;if(ApLowMemory::Build(i,0x8000,h))return 4;std::printf("PASS ap_low_cr3 max=0xffffffff high_cr3_reject=1\n");}
