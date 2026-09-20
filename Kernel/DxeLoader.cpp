#include "DxeLoader.hpp"
#include "Dxe2.hpp"
#include "DxeImports.hpp"
#include "SecurityPolicy.hpp"
namespace Davis::DxeLoader {
static void copy(u8*d,const u8*s,u64 n){for(u64 i=0;i<n;i++)d[i]=s[i];}
static void zero(u8*d,u64 n){for(u64 i=0;i<n;i++)d[i]=0;}
static bool pagesFor(u64 n,u64& p){if(n>~0ull-4095)return false;p=(n+4095)/4096;return true;}
static bool mapped(const Paging::Space&s,u64 va,bool writable,bool executable){u64 pa=0,pte=0;if(!Paging::Translate(s,va,&pa,&pte))return false;if(!(pte&Paging::P)||!(pte&Paging::U))return false;if(((pte&Paging::W)!=0)!=writable)return false;if(((pte&Paging::NX)==0)!=executable)return false;return true;}
bool VerifyUserLayout(const Loaded&o,const Dxe::Header&h){
 if(!o.space.ready||!o.process||!o.codePages||!o.stackPages)return false;
 if(!mapped(o.space,o.codeVa,false,true)||!mapped(o.space,o.codeVa+(o.codePages-1)*4096,false,true))return false;
 if(o.dataPages&&(!mapped(o.space,o.dataVa,true,false)||!mapped(o.space,o.dataVa+(o.dataPages-1)*4096,true,false)))return false;
 if(!mapped(o.space,o.stackBase,true,false)||!mapped(o.space,o.stackTop-4096,true,false))return false;
 u64 x=0;if(Paging::Translate(o.space,o.stackBase-4096,&x,nullptr))return false;
 const u64 entry=o.codeVa+(h.entryOffset-h.codeOffset);return entry>=o.codeVa&&entry<o.codeVa+h.codeBytes&&o.process->entryVirtual==entry&&o.process->userStackTop==o.stackTop;
}
Result Prepare(Process::Table&pt,Memory::PageAllocator&a,const void*img,u64 bytes,Loaded&o){
 if(img&&bytes>=4&&*(const u32*)img==Dxe2::Magic)return PrepareDxe2(pt,a,img,bytes,o);
 o={};const Dxe::Header*h=nullptr;if(Dxe::Validate(img,bytes,&h)!=Dxe::ValidateResult::Ok)return Result::InvalidDxe;if(Paging::Create(o.space,a)!=Paging::Result::Ok)return Result::OutOfMemory;
 u64 dataTotal=0;if(h->dataBytes>~0ull-h->bssBytes)return Result::LayoutOverflow;dataTotal=h->dataBytes+h->bssBytes;
 if(!pagesFor(h->codeBytes,o.codePages)||!pagesFor(dataTotal,o.dataPages)||!pagesFor(h->requiredStackBytes?h->requiredStackBytes:65536,o.stackPages))return Result::LayoutOverflow;
 o.codeVa=0x0000000040000000ull;o.dataVa=0x0000000050000000ull;o.stackBase=0x0000000070001000ull;o.stackTop=o.stackBase+o.stackPages*4096;if(o.stackTop<o.stackBase)return Result::LayoutOverflow;
 u64 cp=Memory::AllocPages(a,o.codePages,1);if(!cp)return Result::OutOfMemory;zero((u8*)(uptr)cp,o.codePages*4096);copy((u8*)(uptr)cp,(const u8*)img+h->codeOffset,h->codeBytes);
 Vm::Region cr{o.codeVa,cp,o.codePages,Vm::Present|Vm::User};if(Paging::MapRegion(o.space,a,cr)!=Paging::Result::Ok)return Result::MapFailed;
 if(o.dataPages){u64 dp=Memory::AllocPages(a,o.dataPages,1);if(!dp)return Result::OutOfMemory;zero((u8*)(uptr)dp,o.dataPages*4096);if(h->dataBytes)copy((u8*)(uptr)dp,(const u8*)img+h->dataOffset,h->dataBytes);Vm::Region dr{o.dataVa,dp,o.dataPages,Vm::Present|Vm::User|Vm::Writable|Vm::NoExecute};if(Paging::MapRegion(o.space,a,dr)!=Paging::Result::Ok)return Result::MapFailed;}
 u64 sp=Memory::AllocPages(a,o.stackPages,1);if(!sp)return Result::OutOfMemory;zero((u8*)(uptr)sp,o.stackPages*4096);Vm::Region sr{o.stackBase,sp,o.stackPages,Vm::Present|Vm::User|Vm::Writable|Vm::NoExecute};if(Paging::MapRegion(o.space,a,sr)!=Paging::Result::Ok)return Result::MapFailed;
 const u64 entry=o.codeVa+(h->entryOffset-h->codeOffset);o.process=Process::CreateValidated(pt,img,bytes,o.codeVa-h->codeOffset,o.stackTop);if(!o.process)return Result::NoProcess;o.process->entryVirtual=entry;o.process->addressSpaceCr3=o.space.pml4Physical;o.process->state=Process::State::Ready;
 if(!VerifyUserLayout(o,*h)){o.process->state=Process::State::Faulted;return Result::VerificationFailed;}return Result::Ok;
}
}

namespace Davis::DxeLoader {
Result PrepareDxe2(Process::Table&pt,Memory::PageAllocator&a,const void*img,u64 bytes,Loaded&o){SecurityDomain::Kind domain=SecurityDomain::Kind::Application;return PrepareDxe2AsDomain(pt,a,img,bytes,o,domain);}
Result PrepareDxe2AsDomain(Process::Table&pt,Memory::PageAllocator&a,const void*img,u64 bytes,Loaded&o,SecurityDomain::Kind domain){
 o={};const Dxe2::Header*h=nullptr;if(Dxe2::Validate(img,bytes,&h)!=Dxe2::ValidateResult::Ok)return Result::InvalidDxe;
 if(Paging::Create(o.space,a)!=Paging::Result::Ok)return Result::OutOfMemory;
 const auto*seg=(const Dxe2::Segment*)((const u8*)img+h->segmentTableOffset);
 for(u32 i=0;i<h->segmentCount;i++){
  const auto&x=seg[i];u64 pages=0;if(!pagesFor(x.memoryBytes,pages))return Result::LayoutOverflow;u64 pa=Memory::AllocPages(a,pages,1);if(!pa)return Result::OutOfMemory;
  zero((u8*)(uptr)pa,pages*4096);if(x.fileBytes)copy((u8*)(uptr)pa,(const u8*)img+x.fileOffset,x.fileBytes);
  u64 f=Vm::Present|Vm::User;if(x.flags&Dxe2::Write)f|=Vm::Writable;if(!(x.flags&Dxe2::Execute))f|=Vm::NoExecute;
  Vm::Region r{x.virtualAddress,pa,pages,f};if(Paging::MapRegion(o.space,a,r)!=Paging::Result::Ok)return Result::MapFailed;
  if(x.flags&Dxe2::Execute){o.codeVa=x.virtualAddress;o.codePages=pages;}else if(!o.dataPages){o.dataVa=x.virtualAddress;o.dataPages=pages;}
 }
 u64 stackBytes=h->base.requiredStackBytes?h->base.requiredStackBytes:65536;if(!pagesFor(stackBytes,o.stackPages))return Result::LayoutOverflow;
 o.stackBase=0x0000000070001000ull;o.stackTop=o.stackBase+o.stackPages*4096;if(o.stackTop<o.stackBase)return Result::LayoutOverflow;
 u64 sp=Memory::AllocPages(a,o.stackPages,1);if(!sp)return Result::OutOfMemory;zero((u8*)(uptr)sp,o.stackPages*4096);Vm::Region sr{o.stackBase,sp,o.stackPages,Vm::Present|Vm::User|Vm::Writable|Vm::NoExecute};if(Paging::MapRegion(o.space,a,sr)!=Paging::Result::Ok)return Result::MapFailed;
 // DXE2 entryOffset is a virtual entry address by design. Process stores the common header prefix.
 for(usize i=0;i<Process::MaxProcesses;i++)if(pt.process[i].state==Process::State::Empty){auto&p=pt.process[i];p={};p.id=pt.nextId++;p.state=Process::State::Ready;p.header=&h->base;p.fileImage=(const u8*)img;p.fileBytes=bytes;p.entryVirtual=h->base.entryOffset;p.userStackTop=o.stackTop;p.dxe2=true;p.domain=domain;p.requestedCapabilities=h->capabilities;p.grantedCapabilities=SecurityPolicy::GrantFor(p.domain,h->capabilities);p.addressSpaceCr3=o.space.pml4Physical;o.process=&p;break;}
 if(!o.process)return Result::NoProcess;u64 pa=0,pte=0;if(!Paging::Translate(o.space,o.process->entryVirtual,&pa,&pte)||!(pte&Paging::U)||(pte&Paging::W)||(pte&Paging::NX)){o.process->state=Process::State::Faulted;return Result::VerificationFailed;}
 if(DxeImports::Bind(*h,img,o.space)!=DxeImports::Result::Ok){o.process->state=Process::State::Faulted;return Result::VerificationFailed;}
 if(Paging::Translate(o.space,o.stackBase-4096,&pa,nullptr)){o.process->state=Process::State::Faulted;return Result::VerificationFailed;}return Result::Ok;
}
}
