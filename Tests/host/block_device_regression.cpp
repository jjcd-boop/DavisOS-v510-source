#include "Kernel/BlockDevice.hpp"
#include <cstdio>
#include <cstring>
using namespace Davis;
static bool finish(u64 id){for(int i=0;i<4;i++)BlockDevice::PumpOnce();i32 e=0;bool ok=BlockDevice::Query(id,&e)==BlockDevice::Status::Complete&&e==0;return ok&&BlockDevice::Reap(id,&e);}
int main(){BlockDevice::Init();unsigned char w[512],r[512];for(unsigned i=0;i<512;i++)w[i]=(unsigned char)(i^0xA5);for(int cycle=0;cycle<10000;cycle++){u64 lba=(u64)(cycle%BlockDevice::RamDiskSectors);w[0]=(unsigned char)cycle;auto a=BlockDevice::Submit(1,BlockDevice::Op::Write,lba,1,w);if(!a||!finish(a))return 1;memset(r,0,sizeof(r));auto b=BlockDevice::Submit(1,BlockDevice::Op::Read,lba,1,r);if(!b||!finish(b)||memcmp(w,r,512))return 2;if((cycle%97)==0){auto f=BlockDevice::Submit(1,BlockDevice::Op::Flush,999,999,(u8*)1);if(!f||!finish(f))return 3;}}
 if(BlockDevice::Submit(1,BlockDevice::Op::Read,BlockDevice::RamDiskSectors,1,r)!=0)return 4;if(BlockDevice::Submit(1,BlockDevice::Op::Write,0,0,w)!=0)return 5;printf("PASS block_device cycles=10000 flush=104 integrity=ok\n");return 0;}
