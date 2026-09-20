#include "../Kernel/PhysicalMemory.hpp"
#include <cstdio>
int main(){
 using namespace Davis; using namespace Davis::Memory;
 alignas(8) unsigned char map[3*48]={};
 struct D{u32 type,pad;u64 ps,vs,np,attr;};
 auto*d0=(D*)map;d0->type=7;d0->ps=0x100000;d0->np=256;
 auto*d1=(D*)(map+48);d1->type=6;d1->ps=0x300000;d1->np=32;
 auto*d2=(D*)(map+96);d2->type=7;d2->ps=0x4000000;d2->np=1024;
 BootInfo b{};b.memoryMap=(u64)(uptr)map;b.memoryMapBytes=sizeof(map);b.descriptorSize=48;
 PageAllocator a;Init(a,b);
 u64 p=AllocPages(a,4,4);if(!p||p%16384)return 1;
 if(FreePages(a,p,4)!=true)return 2;
 if(FreePages(a,p,4)!=false)return 3; // double-free rejection
 u64 q=AllocPages(a,8,16);if(!q||q%65536)return 4;
 std::printf("PASS p=%llx q=%llx free=%llu\n",(unsigned long long)p,(unsigned long long)q,(unsigned long long)a.stats.freePages);
 return 0;
}
