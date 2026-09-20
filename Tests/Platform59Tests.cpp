#define DAVIS_HOST_TEST 1
#include "../Kernel/Faults.hpp"
#include "../Kernel/SyscallAbi.hpp"
#include <cstdio>
int main(){using namespace Davis; if(!Faults::IsUserCodeSelector(0x23)||Faults::IsUserCodeSelector(0x08))return 1;Process::Image p{};p.state=Process::State::Running;Faults::Report f{Faults::Kind::PageFault,0,0x40000000,0xdeadbeef,true};Faults::Contain(&p,f);if(p.state!=Process::State::Faulted)return 2;SyscallAbi::Registers r{2,1,2,3,4,5,6};auto sf=SyscallAbi::Decode(r);if(sf.number!=2||sf.arg3!=4)return 3;auto out=Syscall::Dispatch(sf);SyscallAbi::Encode(out,r);if(r.rax!=0||r.rdx!=0)return 4;std::puts("PASS v0.59 platform boundary tests");}
