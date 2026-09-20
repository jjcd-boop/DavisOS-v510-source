#include "../Kernel/PageTables.hpp"
#include <cstdio>
#include <cstdlib>
int main(){using namespace Davis;void*mem=nullptr;if(posix_memalign(&mem,4096,4096*16))return 1;Memory::PageAllocator a{};a.freeRanges[0]={(u64)(uptr)mem,16};a.rangeCount=1;a.stats.freePages=16;a.stats.totalPages=16;Paging::Space s{};if(Paging::Create(s,a)!=Paging::Result::Ok)return 2;u64 pa=Memory::AllocPages(a,1);if(Paging::Map4K(s,a,Vm::UserBase,pa,Vm::NoExecute)!=Paging::Result::Ok)return 3;u64 got=0,e=0;if(!Paging::Translate(s,Vm::UserBase+123,&got,&e)||got!=pa+123)return 4;if(!(e&Paging::NX)||!(e&Paging::U)||(e&Paging::W))return 5;if(Paging::Map4K(s,a,Vm::UserBase,pa,0)!=Paging::Result::AlreadyMapped)return 6;std::puts("PASS Davis private 4-level paging tests");free(mem);}
