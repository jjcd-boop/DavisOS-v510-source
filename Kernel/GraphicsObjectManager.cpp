#include "GraphicsObjectManager.hpp"
#include "KernelMemory.hpp"
#include "PhysicalMemory.hpp"
namespace Davis::GraphicsObjectManager {
State state{};
static void zero(void*p,u64 n){auto*b=(u8*)p;for(u64 i=0;i<n;i++)b[i]=0;}
static void copyRows(u32*dst,u32 dw,const u32*src,u32 sw,u32 w,u32 h){for(u32 y=0;y<h;y++)for(u32 x=0;x<w;x++)dst[(u64)y*dw+x]=src[(u64)y*sw+x];}
void Init(){state={};state.nextId=0x47000000ull;}
u64 BytesForOwner(u64 owner){u64 n=0;for(auto&o:state.objects)if(o.active&&o.owner==owner)n+=o.bytes;return n;}
Object* Create(u64 owner,u32 w,u32 h,u32 rights){if(!owner||!w||!h||!KernelMemory::ready)return nullptr;u64 pixels=(u64)w*h;if(pixels>0x3fffffffull)return nullptr;u64 bytes=pixels*4ull,pages=(bytes+4095)/4096,charged=pages*4096;if(charged>PerProcessQuotaBytes||BytesForOwner(owner)>PerProcessQuotaBytes-charged||state.totalBytes>GlobalQuotaBytes-charged){state.denied++;return nullptr;}Object*slot=nullptr;for(auto&o:state.objects)if(!o.active){slot=&o;break;}if(!slot){state.denied++;return nullptr;}u64 pa=Memory::AllocPages(KernelMemory::allocator,pages,1);if(!pa){state.denied++;return nullptr;}zero((void*)(uptr)pa,charged);*slot={++state.nextId,owner,pa,pages,charged,0,w,h,w,1,rights,1,true};state.totalBytes+=charged;state.allocations++;return slot;}
Object* Find(u64 owner,u64 id){for(auto&o:state.objects)if(o.active&&o.owner==owner&&o.id==id)return &o;return nullptr;}
const Object* FindAny(u64 id){for(auto&o:state.objects)if(o.active&&o.id==id)return &o;return nullptr;}
bool Resize(u64 owner,u64 id,u32 w,u32 h){auto*o=Find(owner,id);if(!o||!w||!h)return false;if(o->width==w&&o->height==h)return true;u64 bytes=(u64)w*h*4ull,pages=(bytes+4095)/4096,charged=pages*4096,owned=BytesForOwner(owner);if(charged>PerProcessQuotaBytes||owned-o->bytes>PerProcessQuotaBytes-charged||state.totalBytes-o->bytes>GlobalQuotaBytes-charged){state.denied++;return false;}u64 pa=Memory::AllocPages(KernelMemory::allocator,pages,1);if(!pa){state.denied++;return false;}zero((void*)(uptr)pa,charged);u32 cw=w<o->width?w:o->width,ch=h<o->height?h:o->height;copyRows((u32*)(uptr)pa,w,(const u32*)(uptr)o->physical,o->stride,cw,ch);Memory::FreePages(KernelMemory::allocator,o->physical,o->pages);state.totalBytes=state.totalBytes-o->bytes+charged;o->physical=pa;o->pages=pages;o->bytes=charged;o->width=w;o->height=h;o->stride=w;o->generation++;return true;}
void Release(u64 owner,u64 id){auto*o=Find(owner,id);if(!o)return;if(KernelMemory::ready&&o->physical&&o->pages)Memory::FreePages(KernelMemory::allocator,o->physical,o->pages);if(state.totalBytes>=o->bytes)state.totalBytes-=o->bytes;*o={};state.frees++;}
void RevokeProcess(u64 owner){for(usize i=0;i<MaxObjects;i++)if(state.objects[i].active&&state.objects[i].owner==owner){u64 id=state.objects[i].id;Release(owner,id);}}
}
