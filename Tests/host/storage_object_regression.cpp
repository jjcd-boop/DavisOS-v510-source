#include "Kernel/DxeServices.hpp"
#include "Kernel/KernelMemory.hpp"
#include <cstdio>
#include <cstring>
using namespace Davis;
namespace Davis::KernelMemory { Memory::PageAllocator allocator{}; bool ready=false; }
int main(){DxeServices::Init();const u64 owner=77;u32 rights=0;auto h=DxeServices::OpenFileObject(owner,"/Home/Documents/stress.bin",2|4|8,&rights);if(!h)return 1;unsigned char src[4096],dst[4096];for(int i=0;i<4096;i++)src[i]=(unsigned char)(i*31);for(int round=0;round<16;round++)if(DxeServices::WriteFileObject(owner,h,src,sizeof(src))<0)return 2; // bounded at max size
 if(!DxeServices::CloseFileObject(owner,h))return 3;h=DxeServices::OpenFileObject(owner,"/Home/Documents/stress.bin",1,&rights);if(!h)return 4;auto n=DxeServices::ReadFileObject(owner,h,dst,sizeof(dst));if(n<=0||std::memcmp(src,dst,(size_t)n)!=0)return 5;
 DxeServices::RevokeProcess(owner);if(DxeServices::FindOpenFile(owner,h))return 6;u64 f,o,r;DxeServices::StorageMaintenance(f,o,r);if(f<1)return 7;std::printf("PASS storage_object bounded-write read-integrity revoke maintenance files=%llu\n",(unsigned long long)f);return 0;}
