#include "Kernel/Ipc.hpp"
#include "Kernel/ObjectHandles.hpp"
#include "Kernel/SystemServices.hpp"
#include "Kernel/DxeServices.hpp"
#include "Kernel/KernelMemory.hpp"
#include <cstdio>
using namespace Davis;
namespace Davis::KernelMemory { Memory::PageAllocator allocator{}; bool ready=false; }
static int activeHandles(const ObjectHandles::Table&t,u64 owner){int n=0;for(auto&e:t.entries)if(e.active&&e.ownerProcessId==owner)n++;return n;}
int main(){
 Ipc::Init(); SystemServices::Init(); DxeServices::Init(); ObjectHandles::Table ht{}; ObjectHandles::Init(ht);
 const u64 survivor=9001; auto sep=Ipc::CreateEndpoint(survivor,Ipc::RightSend|Ipc::RightReceive); if(!sep)return 1;
 auto sh=ObjectHandles::Create(ht,survivor,ObjectHandles::Type::IpcEndpoint,sep,ObjectHandles::RightSend|ObjectHandles::RightReceive|ObjectHandles::RightClose); if(!sh)return 2;
 for(u64 round=1;round<=1000;round++){
  u64 victim=10000+round; auto ep=Ipc::CreateEndpoint(victim,Ipc::RightSend|Ipc::RightReceive); if(!ep)return 3;
  auto h=ObjectHandles::Create(ht,victim,ObjectHandles::Type::IpcEndpoint,ep,ObjectHandles::RightSend|ObjectHandles::RightReceive|ObjectHandles::RightClose); if(!h)return 4;
  if(!SystemServices::Register(70,victim,ep))return 5;
  u32 rights=0;auto f=DxeServices::OpenFileObject(victim,"/Home/Documents/teardown.tmp",2|4|8,&rights);if(!f)return 6;
  const char b[]="isolation";if(DxeServices::WriteFileObject(victim,f,b,sizeof(b))<0)return 7;
  Ipc::RevokeProcess(victim);SystemServices::UnregisterOwner(victim);DxeServices::RevokeProcess(victim);ObjectHandles::RevokeProcess(ht,victim);
  if(Ipc::OwnerOf(ep)!=0||SystemServices::Lookup(70)!=0||DxeServices::FindOpenFile(victim,f)||activeHandles(ht,victim))return 8;
  if(Ipc::OwnerOf(sep)!=survivor||!ObjectHandles::Resolve(ht,survivor,sh,ObjectHandles::Type::IpcEndpoint,ObjectHandles::RightReceive))return 9;
 }
 std::printf("PASS resource_teardown cycles=1000 survivor_isolated=1\n");return 0;
}
