#include "PhysicalMemory.hpp"
namespace Davis::Memory {
struct EfiDesc { u32 type;u32 pad;u64 physicalStart;u64 virtualStart;u64 numberOfPages;u64 attribute; };
static constexpr u64 MaxU64=~0ull;
static bool addOk(u64 a,u64 b,u64&out){if(a>MaxU64-b)return false;out=a+b;return true;}
static bool mulOk(u64 a,u64 b,u64&out){if(a&&b>MaxU64/a)return false;out=a*b;return true;}
static bool pow2(u64 v){return v&&!(v&(v-1));}
static bool span(u64 base,u64 pages,u64&end){u64 bytes=0;return mulOk(pages,PageSize,bytes)&&addOk(base,bytes,end);}
static bool overlap(u64 a,u64 an,u64 b,u64 bn){u64 ae=0,be=0;if(!addOk(a,an,ae)||!addOk(b,bn,be))return true;return a<be&&b<ae;}
bool IsPageAligned(u64 v){return (v&(PageSize-1))==0;}
static void remove(PageAllocator&a,usize i){for(usize j=i+1;j<a.rangeCount;j++)a.freeRanges[j-1]=a.freeRanges[j];a.rangeCount--;}
static bool insert(PageAllocator&a,u64 base,u64 pages){
 if(!pages||!IsPageAligned(base)||a.rangeCount>=PageAllocator::MaxRanges)return false;u64 end=0;if(!span(base,pages,end))return false;
 usize i=0;while(i<a.rangeCount&&a.freeRanges[i].base<base)i++;
 if(i){u64 pe=0;if(!span(a.freeRanges[i-1].base,a.freeRanges[i-1].pages,pe)||base<pe)return false;}
 if(i<a.rangeCount&&end>a.freeRanges[i].base)return false;
 for(usize j=a.rangeCount;j>i;j--)a.freeRanges[j]=a.freeRanges[j-1];a.freeRanges[i]={base,pages};a.rangeCount++;
 for(usize j=0;j+1<a.rangeCount;){u64 xe=0;auto&x=a.freeRanges[j];auto&y=a.freeRanges[j+1];if(span(x.base,x.pages,xe)&&xe==y.base){u64 sum=0;if(addOk(x.pages,y.pages,sum)){x.pages=sum;remove(a,j+1);continue;}}j++;}return true;
}
void Reserve(PageAllocator&a,u64 base,u64 bytes){
 if(!bytes)return;u64 rawHi=0;if(!addOk(base,bytes,rawHi))rawHi=MaxU64;u64 lo=base&~(PageSize-1),hi=rawHi;if(hi!=MaxU64){u64 t=0;if(!addOk(hi,PageSize-1,t))hi=MaxU64;else hi=t&~(PageSize-1);}if(hi<=lo)return;
 for(usize i=0;i<a.rangeCount;){auto r=a.freeRanges[i];u64 re=0;if(!span(r.base,r.pages,re)){remove(a,i);continue;}if(!overlap(r.base,re-r.base,lo,hi-lo)){i++;continue;}remove(a,i);if(lo>r.base)(void)insert(a,r.base,(lo-r.base)/PageSize);if(hi<re)(void)insert(a,hi,(re-hi)/PageSize);}
}
void Init(PageAllocator&a,const BootInfo&b){
 a={};if(!b.memoryMap||!b.memoryMapBytes||b.descriptorSize<sizeof(EfiDesc))return;u8*p=(u8*)(uptr)b.memoryMap;
 for(u64 off=0;off<=b.memoryMapBytes-b.descriptorSize;off+=b.descriptorSize){auto*d=(EfiDesc*)(p+off);if(d->type!=7||!d->numberOfPages||!IsPageAligned(d->physicalStart))continue;u64 end=0;if(!span(d->physicalStart,d->numberOfPages,end))continue;u64 total=0;if(addOk(a.stats.totalPages,d->numberOfPages,total))a.stats.totalPages=total;if(!insert(a,d->physicalStart,d->numberOfPages))continue;}
 Reserve(a,0,0x100000);Reserve(a,0x02000000,64*1024*1024);
 u64 fbBytes=0,fbPixels=0;if(mulOk((u64)b.framebuffer.pixelsPerScanLine,(u64)b.framebuffer.height,fbPixels)&&mulOk(fbPixels,4,fbBytes))Reserve(a,(u64)b.framebuffer.base,fbBytes);
 Reserve(a,(u64)(uptr)&b,sizeof(BootInfo));Reserve(a,b.memoryMap,b.memoryMapBytes);
 u64 free=0;for(usize i=0;i<a.rangeCount;i++){u64 t=0;if(!addOk(free,a.freeRanges[i].pages,t)){free=MaxU64;break;}free=t;}a.stats.freePages=free;a.stats.reservedPages=a.stats.totalPages>free?a.stats.totalPages-free:0;
}
u64 AllocPages(PageAllocator&a,u64 pages,u64 align){
 if(!pages)return 0;if(!align)align=1;if(!pow2(align))return 0;u64 al=0,need=0;if(!mulOk(align,PageSize,al)||!mulOk(pages,PageSize,need))return 0;
 for(usize i=0;i<a.rangeCount;i++){auto r=a.freeRanges[i];u64 re=0;if(!span(r.base,r.pages,re))continue;u64 plus=0;if(!addOk(r.base,al-1,plus))continue;u64 start=plus&~(al-1),end=0;if(!addOk(start,need,end)||end>re)continue;remove(a,i);if(start>r.base)(void)insert(a,r.base,(start-r.base)/PageSize);if(end<re)(void)insert(a,end,(re-end)/PageSize);if(a.stats.freePages>=pages)a.stats.freePages-=pages;else a.stats.freePages=0;u64 t=0;if(addOk(a.stats.allocatedPages,pages,t))a.stats.allocatedPages=t;return start;}return 0;
}
bool FreePages(PageAllocator&a,u64 base,u64 pages){
 if(!pages||!IsPageAligned(base))return false;u64 end=0;if(!span(base,pages,end))return false;for(usize i=0;i<a.rangeCount;i++){u64 re=0;if(!span(a.freeRanges[i].base,a.freeRanges[i].pages,re))return false;if(overlap(base,end-base,a.freeRanges[i].base,re-a.freeRanges[i].base))return false;}if(!insert(a,base,pages))return false;u64 t=0;if(addOk(a.stats.freePages,pages,t))a.stats.freePages=t;if(a.stats.allocatedPages>=pages)a.stats.allocatedPages-=pages;else a.stats.allocatedPages=0;return true;
}
}
