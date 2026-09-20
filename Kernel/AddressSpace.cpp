#include "AddressSpace.hpp"
namespace Davis::AddressSpace {
extern "C" { extern char __text_start[],__text_end[],__rodata_start[],__rodata_end[],__data_start[],__data_end[],__bss_start[],__bss_end[]; }
static u64 sectionFlags(u64 va){
 const u64 ts=(u64)(uptr)__text_start,te=(u64)(uptr)__text_end,rs=(u64)(uptr)__rodata_start,re=(u64)(uptr)__rodata_end,ds=(u64)(uptr)__data_start,de=(u64)(uptr)__data_end,bs=(u64)(uptr)__bss_start,be=(u64)(uptr)__bss_end;
 if(va>=ts&&va<te)return 0;                 // kernel text: read + execute
 if(va>=rs&&va<re)return Paging::NX;        // constants: read-only + NX
 if((va>=ds&&va<de)||(va>=bs&&va<be))return Paging::W|Paging::NX; // mutable data
 return Paging::W|Paging::NX;               // reserved kernel-image slack: never executable
}

static bool supervisorPage(const Paging::Space&s,u64 va,u64 expectedPa){u64 pa=0,e=0;if(!Paging::Translate(s,va,&pa,&e))return false;return (pa&~0xfffull)==(expectedPa&~0xfffull) && !(e&Paging::U);}
Result InstallKernelContract(Paging::Space&s,Memory::PageAllocator&a,u64 rsp0Base,u64 rsp0Pages){
 if(!s.ready||!rsp0Base||!rsp0Pages||(rsp0Base&4095))return Result::Invalid;
 for(u64 off=0;off<KernelImageSpan;off+=4096){auto r=Paging::MapKernel4K(s,a,KernelImageBase+off,KernelImageBase+off,sectionFlags(KernelImageBase+off));if(r!=Paging::Result::Ok&&r!=Paging::Result::AlreadyMapped)return Result::MapFailed;}
 for(u64 i=0;i<rsp0Pages;i++){u64 p=rsp0Base+i*4096;auto r=Paging::MapKernel4K(s,a,p,p,Paging::W|Paging::NX);if(r!=Paging::Result::Ok&&r!=Paging::Result::AlreadyMapped)return Result::MapFailed;}
 return VerifyKernelContract(s,rsp0Base,rsp0Pages)?Result::Ok:Result::ContractFailed;
}
bool VerifyKernelContract(const Paging::Space&s,u64 rsp0Base,u64 rsp0Pages){
 if(!s.ready||!rsp0Base||!rsp0Pages||(rsp0Base&4095))return false;
 // Check every kernel-image page, not merely the first PML4 entry: a missing
 // page in an otherwise shared hierarchy must fail the contract.
 for(u64 off=0;off<KernelImageSpan;off+=4096)if(!supervisorPage(s,KernelImageBase+off,KernelImageBase+off))return false;
 for(u64 i=0;i<rsp0Pages;i++){u64 p=rsp0Base+i*4096;if(!supervisorPage(s,p,p))return false;}
 return true;
}
Result Create(State&s,Memory::PageAllocator&a){s={};if(Paging::Create(s.space,a)!=Paging::Result::Ok)return Result::OutOfMemory;
 u64 sp=Memory::AllocPages(a,KernelStackPages,1);if(!sp)return Result::OutOfMemory;
 // The canonical AddressSpace object keeps its kernel stack at a stable VA.
 for(u64 off=0;off<KernelImageSpan;off+=4096){auto r=Paging::MapKernel4K(s.space,a,KernelImageBase+off,KernelImageBase+off,Paging::W);if(r!=Paging::Result::Ok)return Result::MapFailed;}
 for(u64 i=0;i<KernelStackPages;i++){auto r=Paging::MapKernel4K(s.space,a,KernelStackVa+i*4096,sp+i*4096,Paging::W|Paging::NX);if(r!=Paging::Result::Ok)return Result::MapFailed;}
 s.kernelStackPhysical=sp;s.kernelStackTop=KernelStackVa+KernelStackPages*4096;s.ready=true;
 // Stable-VA stacks are verified directly because physical != virtual here.
 for(u64 off=0;off<KernelImageSpan;off+=4096)if(!supervisorPage(s.space,KernelImageBase+off,KernelImageBase+off))return Result::ContractFailed;
 for(u64 i=0;i<KernelStackPages;i++)if(!supervisorPage(s.space,KernelStackVa+i*4096,sp+i*4096))return Result::ContractFailed;
 s.sharedKernelMappings=true;return Result::Ok;}
}
