#include "../../Kernel/BlockDevice.hpp"
#include <cstdio>
using namespace Davis; using namespace Davis::BlockDevice;
static u64 submits=0,polls=0; static bool injectFail=false; static u8 disk[4096*512]{};
static bool sub(const Device&,Request&r){submits++;r.backendToken=2;return true;}
static bool poll(const Device&d,Request&r,bool&done,bool&ok,i32&err){polls++;done=true;ok=!injectFail;err=ok?0:-99;if(!ok)return true;if(r.op==Op::Flush)return true;u64 off=r.lba*d.sectorBytes,n=(u64)r.sectorCount*d.sectorBytes;for(u64 i=0;i<n;i++){if(r.op==Op::Write)disk[off+i]=r.buffer[i];else r.buffer[i]=disk[off+i];}return true;}
static bool finish(u64 id,bool expect=true){for(int i=0;i<8;i++)PumpOnce();i32 e=0;auto s=Query(id,&e);bool ok=(s==Status::Complete);if(ok!=expect)return false;return Reap(id,&e);}
int main(){Init();if(!RegisterBackend(2,sub,poll)||!RegisterDevice(2,4096,512,true,2))return 1;if(RegisterBackend(2,sub,poll)||RegisterDevice(2,1,512,true,2))return 2;
 u8 w[1024],r[1024];for(int cycle=0;cycle<20000;cycle++){for(int i=0;i<1024;i++)w[i]=(u8)(cycle+i);u64 lba=(u64)(cycle%(4096-2));auto a=Submit(2,Op::Write,lba,2,w,16);if(!a||!finish(a))return 3;for(auto&x:r)x=0;auto b=Submit(2,Op::Read,lba,2,r,16);if(!b||!finish(b))return 4;for(int i=0;i<1024;i++)if(r[i]!=w[i])return 5;if((cycle%257)==0){auto f=Submit(2,Op::Flush,999,999,(u8*)1,16);if(!f||!finish(f))return 6;}}
 injectFail=true;auto bad=Submit(2,Op::Read,0,1,r,16);if(!bad||!finish(bad,false))return 7;injectFail=false;if(Submit(2,Op::Read,4096,1,r,16)!=0)return 8;
 std::printf("PASS block_backend cycles=20000 submits=%llu polls=%llu completed=%llu failed=%llu\n",(unsigned long long)submits,(unsigned long long)polls,(unsigned long long)state.completed,(unsigned long long)state.failed);return 0;}
