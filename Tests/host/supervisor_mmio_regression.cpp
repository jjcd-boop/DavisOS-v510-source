#include "Kernel/PageTables.hpp"
#include "Kernel/DeviceMappings.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstdint>
using namespace Davis;
int main(){
 constexpr size_t pages=64, bytes=pages*4096; void*mem=nullptr;
 if(posix_memalign(&mem,4096,bytes)!=0||!mem)return 2;
 auto base=(u64)(uintptr_t)mem; Memory::PageAllocator a{};a.freeRanges[0]={base,pages};a.rangeCount=1;a.stats.freePages=pages;
 Paging::Space s{};if(Paging::Create(s,a)!=Paging::Result::Ok)return 3;
 constexpr u64 lapic=0xFEE00000ull;
 if(DeviceMappings::InstallSupervisorMmio(s,a,lapic,1)!=DeviceMappings::Result::Ok)return 4;
 u64 pa=0,e=0;if(!Paging::Translate(s,lapic,&pa,&e))return 5;
 if((pa&Paging::AddrMask)!=lapic)return 6;if(e&Paging::U)return 7;if(!(e&Paging::W)||!(e&Paging::NX))return 8;
 // User mapper must still refuse a collision with the supervisor LAPIC page.
 if(Paging::Map4K(s,a,lapic,base,Paging::W)!=Paging::Result::InvalidAddress)return 9;
 std::printf("PASS supervisor_mmio va=%llx pa=%llx user=0 nx=1 write=1\n",(unsigned long long)lapic,(unsigned long long)(pa&Paging::AddrMask));
 free(mem);return 0;
}
