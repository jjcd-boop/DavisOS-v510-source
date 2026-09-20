#include "Dma.hpp"
namespace Davis::Memory {
DmaBuffer AllocDma(PageAllocator&a,u64 bytes,u64 alignment){
 DmaBuffer d{};if(!bytes)return d;if(alignment<PageSize)alignment=PageSize;
 u64 pages=(bytes+PageSize-1)/PageSize,ap=(alignment+PageSize-1)/PageSize;
 // Identity mapping is the current standalone-kernel contract.
 u64 p=AllocPages(a,pages,ap);if(!p)return d;d={p,(void*)(uptr)p,bytes,pages,alignment};Zero(d);return d;
}
bool FreeDma(PageAllocator&a,DmaBuffer&d){if(!d.physical)return false;bool ok=FreePages(a,d.physical,d.pages);if(ok)d={};return ok;}
void Zero(DmaBuffer&d){u8*p=(u8*)d.virtualAddress;for(u64 i=0;i<d.pages*PageSize;i++)p[i]=0;}
}
