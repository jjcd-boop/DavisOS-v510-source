#include "DeviceMappings.hpp"
namespace Davis::DeviceMappings {
static bool valid(u64 base,u64 pages){return base && !(base&0xfffull) && pages && pages<=256;}
bool VerifySupervisorMmio(const Paging::Space&s,u64 base,u64 pages){
 if(!s.ready||!valid(base,pages))return false;
 for(u64 i=0;i<pages;i++){u64 va=base+i*4096,pa=0,e=0;if(!Paging::Translate(s,va,&pa,&e))return false;if((pa&Paging::AddrMask)!=(va&Paging::AddrMask))return false;if(e&Paging::U)return false;if(!(e&Paging::W))return false;if(!(e&Paging::NX))return false;}
 return true;
}
Result InstallSupervisorMmio(Paging::Space&s,Memory::PageAllocator&a,u64 base,u64 pages){
 if(!s.ready||!valid(base,pages))return Result::Invalid;
 for(u64 i=0;i<pages;i++){u64 p=base+i*4096;auto r=Paging::MapSupervisor4K(s,a,p,p,Paging::W|Paging::NX);if(r!=Paging::Result::Ok&&r!=Paging::Result::AlreadyMapped)return Result::MapFailed;}
 return VerifySupervisorMmio(s,base,pages)?Result::Ok:Result::VerifyFailed;
}
}
