#include <cstdio>
#include <thread>
#include <vector>
#include "Kernel/SmpSync.hpp"
using namespace Davis;
int main(){
 SmpSync::SpinLock lock{};SmpSync::Init(lock);if(SmpSync::IsLocked(lock))return 1;
 if(!SmpSync::TryLock(lock,7)||SmpSync::Owner(lock)!=7)return 2;
 if(SmpSync::TryLock(lock,8))return 3;
 if(SmpSync::Unlock(lock,8))return 4;
 if(!SmpSync::Unlock(lock,7)||SmpSync::IsLocked(lock))return 5;
 constexpr int threads=8, loops=25000; unsigned long long counter=0;
 std::vector<std::thread> v;v.reserve(threads);
 for(int t=0;t<threads;t++)v.emplace_back([&,t](){for(int i=0;i<loops;i++){SmpSync::Lock(lock,(u32)t);auto x=counter;counter=x+1;if(!SmpSync::Unlock(lock,(u32)t))__builtin_trap();}});
 for(auto&t:v)t.join();
 if(counter!=(unsigned long long)threads*loops)return 6;
 if(lock.acquisitions < (unsigned long long)threads*loops+1)return 7;
 std::printf("PASS smp_sync threads=%d acquisitions=%llu contentions=%llu counter=%llu\n",threads,(unsigned long long)lock.acquisitions,(unsigned long long)lock.contentions,counter);return 0;
}
