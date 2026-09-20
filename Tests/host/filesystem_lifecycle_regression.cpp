#include "Kernel/DxeServices.hpp"
#include "Kernel/KernelMemory.hpp"
#include <cstdio>
#include <cstring>
using namespace Davis;
namespace Davis::KernelMemory { Memory::PageAllocator allocator{}; bool ready=false; }
int main(){
 DxeServices::Init(); u32 rights=0; const u64 owner=101, other=202;
 const char* bad[]={"relative","/Home/../System/x","/Home/./x","/Home/Documents/../../x","/Home\\evil"};
 for(auto p:bad) if(DxeServices::OpenFileObject(owner,p,2|4,&rights)) return 1;
 unsigned char w[1024],r[1024]; for(unsigned i=0;i<sizeof(w);i++)w[i]=(unsigned char)(i*17u+3u);
 for(int cycle=0;cycle<5000;cycle++){
  auto h=DxeServices::OpenFileObject(owner,"/Home/Documents/lifecycle.bin",2|4|8,&rights); if(!h)return 2;
  if(DxeServices::WriteFileObject(owner,h,w,sizeof(w))!=(i64)sizeof(w))return 3;
  if(!DxeServices::SeekFileObject(owner,h,0))return 4;
  if(DxeServices::SeekFileObject(other,h,0))return 5;
  if(!DxeServices::CloseFileObject(owner,h))return 6;
  h=DxeServices::OpenFileObject(owner,"/Home/Documents/lifecycle.bin",1,&rights);if(!h)return 7;
  if(DxeServices::ReadFileObject(owner,h,r,sizeof(r))!=(i64)sizeof(r)||std::memcmp(w,r,sizeof(w)))return 8;
  if(DxeServices::DeleteFileObject(owner,"/Home/Documents/lifecycle.bin"))return 9;
  if(!DxeServices::CloseFileObject(owner,h))return 10;
  if(!DxeServices::RenameFileObject(owner,"/Home/Documents/lifecycle.bin","/Home/Documents/renamed.bin"))return 11;
  if(DxeServices::OpenFileObject(owner,"/Home/Documents/lifecycle.bin",1,&rights))return 12;
  if(!DxeServices::RenameFileObject(owner,"/Home/Documents/renamed.bin","/Home/Documents/lifecycle.bin"))return 13;
 }
 if(!DxeServices::DeleteFileObject(owner,"/Home/Documents/lifecycle.bin"))return 14;
 if(DxeServices::OpenFileObject(owner,"/Home/Documents/lifecycle.bin",1,&rights))return 15;
 std::puts("PASS filesystem_lifecycle path-security seek rename delete cycles=5000"); return 0;
}
