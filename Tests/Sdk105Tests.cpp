#include "../SDK/include/davis/davis.hpp"
#include "../Kernel/Syscall.hpp"
#include <cstdio>
int main(){
 using namespace DavisSDK;
 static_assert(Exit==Davis::Syscall::Exit); static_assert(Yield==Davis::Syscall::Yield); static_assert(CreateWindow==Davis::Syscall::CreateWindow); static_assert(OpenFile==Davis::Syscall::OpenFile); static_assert(CloseFile==Davis::Syscall::CloseFile);
 if(sizeof(Result)!=16){std::puts("FAIL result ABI");return 1;} std::puts("PASS v1.05 Davis SDK syscall ABI"); return 0;
}
