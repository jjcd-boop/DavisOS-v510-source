#include "Kernel/NvmeRecovery.hpp"
#include <cstdio>
using namespace Davis;
struct F{u32 q=0,r=0,i=0;u32 failReset=0,failInit=0;};static bool q(void*p){((F*)p)->q++;return true;}static bool r(void*p){auto*f=(F*)p;f->r++;return f->r>f->failReset;}static bool init(void*p){auto*f=(F*)p;f->i++;return f->i>f->failInit;}
int main(){u64 cycles=0;for(u32 n=0;n<100000;n++){F f{};f.failReset=n%3;NvmeRecovery::State s{};if(!NvmeRecovery::Init(s,4))return 1;NvmeRecovery::Hooks h{q,r,init,&f};if(!NvmeRecovery::Recover(s,h)||s.stage!=NvmeRecovery::Stage::Recovered)return 2;cycles++;}F f{};f.failReset=99;NvmeRecovery::State s{};NvmeRecovery::Init(s,3);NvmeRecovery::Hooks h{q,r,init,&f};if(NvmeRecovery::Recover(s,h)||s.stage!=NvmeRecovery::Stage::Failed||s.attempts!=3)return 3;std::printf("PASS nvme_recovery cycles=%llu bounded_failure_attempts=%u\n",(unsigned long long)cycles,s.attempts);}
