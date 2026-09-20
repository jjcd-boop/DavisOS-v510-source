#include "PageTables.hpp"
namespace Davis::Paging {
void Init(Space&s){s={};}
static u64* table(u64 p){return (u64*)(uptr)p;}
static void zeroPage(u64 p){auto*q=table(p);for(usize i=0;i<512;i++)q[i]=0;}
static bool canonical(u64 v){u64 hi=v>>48;return hi==0||hi==0xffff;}
static u64 activeCr3(){
#if defined(__x86_64__) && !defined(DAVIS_HOST_TEST)
 u64 v;asm volatile("mov %%cr3,%0":"=r"(v));return v&AddrMask;
#else
 return 0;
#endif
}
void FlushPage(u64 va){
#if defined(__x86_64__) && !defined(DAVIS_HOST_TEST)
 asm volatile("invlpg (%0)"::"r"((void*)(uptr)va):"memory");
#else
 (void)va;
#endif
}
void FlushSpaceIfActive(const Space&s){
#if defined(__x86_64__) && !defined(DAVIS_HOST_TEST)
 if((s.pml4Physical&AddrMask)==activeCr3()){u64 c;asm volatile("mov %%cr3,%0":"=r"(c));asm volatile("mov %0,%%cr3"::"r"(c):"memory");}
#else
 (void)s;
#endif
}
Result Create(Space&s,Memory::PageAllocator&a){Init(s);u64 p=Memory::AllocPages(a,1,1);if(!p)return Result::OutOfMemory;zeroPage(p);s.pml4Physical=p;s.ready=true;return Result::Ok;}
static Result next(u64*parent,usize ix,Memory::PageAllocator&a,u64*&out,bool user){u64 e=parent[ix];if(!(e&P)){u64 p=Memory::AllocPages(a,1,1);if(!p)return Result::OutOfMemory;zeroPage(p);parent[ix]=p|P|W|(user?U:0);e=parent[ix];}else if(user&&!(e&U)){return Result::InvalidAddress;}out=table(e&AddrMask);return Result::Ok;}
Result Map4K(Space&s,Memory::PageAllocator&a,u64 va,u64 pa,u64 f){if(!s.ready||!canonical(va)||(va&4095)||(pa&4095)||va<Vm::UserBase||va>=Vm::UserTop)return Result::InvalidAddress;u64*pml4=table(s.pml4Physical),*pdpt,*pd,*pt;Result r;if((r=next(pml4,(va>>39)&511,a,pdpt,true))!=Result::Ok)return r;if((r=next(pdpt,(va>>30)&511,a,pd,true))!=Result::Ok)return r;if((r=next(pd,(va>>21)&511,a,pt,true))!=Result::Ok)return r;usize i=(va>>12)&511;if(pt[i]&P)return Result::AlreadyMapped;u64 pf=P|U;if(f&Vm::Writable)pf|=W;if(f&Vm::NoExecute)pf|=NX;pt[i]=(pa&AddrMask)|pf;s.mappedPages++;return Result::Ok;}
Result MapKernel4K(Space&s,Memory::PageAllocator&a,u64 va,u64 pa,u64 f){if(!s.ready||!canonical(va)||(va&4095)||(pa&4095)||(va>=Vm::UserBase&&va<Vm::UserTop))return Result::InvalidAddress;u64*pml4=table(s.pml4Physical),*pdpt,*pd,*pt;Result r;if((r=next(pml4,(va>>39)&511,a,pdpt,false))!=Result::Ok)return r;if((r=next(pdpt,(va>>30)&511,a,pd,false))!=Result::Ok)return r;if((r=next(pd,(va>>21)&511,a,pt,false))!=Result::Ok)return r;usize i=(va>>12)&511;if(pt[i]&P)return Result::AlreadyMapped;u64 pf=P|(f&W);if(f&NX)pf|=NX;pt[i]=(pa&AddrMask)|pf;s.mappedPages++;return Result::Ok;}
Result MapSupervisor4K(Space&s,Memory::PageAllocator&a,u64 va,u64 pa,u64 f){if(!s.ready||!canonical(va)||(va&4095)||(pa&4095))return Result::InvalidAddress;u64*pml4=table(s.pml4Physical),*pdpt,*pd,*pt;Result r;if((r=next(pml4,(va>>39)&511,a,pdpt,false))!=Result::Ok)return r;if((r=next(pdpt,(va>>30)&511,a,pd,false))!=Result::Ok)return r;if((r=next(pd,(va>>21)&511,a,pt,false))!=Result::Ok)return r;usize i=(va>>12)&511;if(pt[i]&P)return Result::AlreadyMapped;u64 pf=P|(f&W);if(f&NX)pf|=NX;pt[i]=(pa&AddrMask)|pf;s.mappedPages++;return Result::Ok;}
Result Unmap4K(Space&s,u64 va,bool flush){if(!s.ready||!canonical(va)||(va&4095))return Result::InvalidAddress;u64*t=table(s.pml4Physical);u64 e=t[(va>>39)&511];if(!(e&P))return Result::NotMapped;t=table(e&AddrMask);e=t[(va>>30)&511];if(!(e&P))return Result::NotMapped;t=table(e&AddrMask);e=t[(va>>21)&511];if(!(e&P))return Result::NotMapped;t=table(e&AddrMask);usize i=(va>>12)&511;if(!(t[i]&P))return Result::NotMapped;t[i]=0;if(s.mappedPages)s.mappedPages--;if(flush&&(s.pml4Physical&AddrMask)==activeCr3())FlushPage(va);return Result::Ok;}
Result UnmapRegion(Space&s,u64 va,u64 pages,bool flush){if(!pages||pages>(~0ull/4096)||va+pages*4096<va)return Result::InvalidAddress;for(u64 i=0;i<pages;i++){auto r=Unmap4K(s,va+i*4096,flush);if(r!=Result::Ok&&r!=Result::NotMapped)return r;}return Result::Ok;}
Result MapRegion(Space&s,Memory::PageAllocator&a,const Vm::Region&r){if(!r.pages||r.pages>(~0ull/4096)||r.virtualBase+r.pages*4096<r.virtualBase||r.physicalBase+r.pages*4096<r.physicalBase)return Result::InvalidAddress;u64 done=0;for(;done<r.pages;done++){auto x=Map4K(s,a,r.virtualBase+done*4096,r.physicalBase+done*4096,r.flags);if(x!=Result::Ok){while(done){--done;Unmap4K(s,r.virtualBase+done*4096,false);}FlushSpaceIfActive(s);return x;}}return Result::Ok;}
bool Translate(const Space&s,u64 va,u64*po,u64*eo){if(!s.ready||!canonical(va))return false;u64*t=table(s.pml4Physical);u64 e=t[(va>>39)&511];if(!(e&P))return false;t=table(e&AddrMask);e=t[(va>>30)&511];if(!(e&P))return false;t=table(e&AddrMask);e=t[(va>>21)&511];if(!(e&P))return false;t=table(e&AddrMask);e=t[(va>>12)&511];if(!(e&P))return false;if(po)*po=(e&AddrMask)|(va&4095);if(eo)*eo=e;return true;}
}
