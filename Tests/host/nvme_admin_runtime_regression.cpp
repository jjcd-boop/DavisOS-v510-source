#include "Kernel/NvmeAdminRuntime.hpp"
#include <cstdio>
using namespace Davis;
struct Fake{u32 submits=0,polls=0,rings=0,resets=0;bool timeout=false;};
static bool submit(void*c,const Nvme::IoCommand&x){auto*f=(Fake*)c;f->submits++;return x.cid!=0;}
static bool poll(void*c,u16,u16&st){auto*f=(Fake*)c;f->polls++;if(f->timeout)return false;st=0;return true;}
static void ring(void*c,bool,u16){((Fake*)c)->rings++;} static void reset(void*c){((Fake*)c)->resets++;}
int main(){Nvme::QueueCreatePlan p{};if(!Nvme::BuildQueueCreatePlan(1,64,0x100000,0x200000,3,p))return 1;u64 ok=0;for(u32 i=0;i<100000;i++){Fake f{};NvmeAdminRuntime::Transport t{submit,poll,ring,reset,&f};NvmeAdminRuntime::State s{};if(!NvmeAdminRuntime::CreateIoQueues(t,p,1,2,1000,s)||!s.ready||f.submits!=2||f.rings!=4||f.resets)return 2;ok++;}Fake f{};f.timeout=true;NvmeAdminRuntime::Transport t{submit,poll,ring,reset,&f};NvmeAdminRuntime::State s{};if(NvmeAdminRuntime::CreateIoQueues(t,p,1,2,100,s)||s.stage!=NvmeAdminRuntime::Stage::TimedOut||f.resets!=1)return 3;std::printf("PASS nvme_admin_runtime cycles=%llu timeout_reset=1\n",(unsigned long long)ok);}
