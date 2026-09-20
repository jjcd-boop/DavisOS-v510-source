#include "StackGuard.hpp"
namespace Davis::StackGuard {
bool Allocate(Stack&s,Memory::PageAllocator&a,u64 n){s={};if(!n)return false;u64 p=Memory::AllocPages(a,n+1,1);if(!p)return false;s.allocationBase=p;s.guardBase=p;s.usableBase=p+4096;s.top=s.usableBase+n*4096;s.usablePages=n;s.canary=(volatile u64*)s.usableBase;*s.canary=Canary;s.ready=true;return true;}
bool MapInto(const Stack&s,Paging::Space&as,Memory::PageAllocator&a){if(!s.ready||!as.ready)return false;u64 dummy=0;if(Paging::Translate(as,s.guardBase,&dummy,nullptr))return false;for(u64 i=0;i<s.usablePages;i++){u64 p=s.usableBase+i*4096;auto r=Paging::MapKernel4K(as,a,p,p,Paging::W|Paging::NX);if(r!=Paging::Result::Ok&&r!=Paging::Result::AlreadyMapped)return false;}return VerifyMapped(s,as);}
bool VerifyMapped(const Stack&s,const Paging::Space&as){u64 p=0,e=0;if(Paging::Translate(as,s.guardBase,&p,&e))return false;for(u64 i=0;i<s.usablePages;i++){u64 v=s.usableBase+i*4096;if(!Paging::Translate(as,v,&p,&e)||((p&~0xfffull)!=v)||(e&Paging::U))return false;}return true;}
bool CanaryIntact(const Stack&s){return s.ready&&s.canary&&*s.canary==Canary;}
}
