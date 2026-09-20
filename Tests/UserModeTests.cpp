#define DAVIS_HOST_TEST 1
#include "../Kernel/UserMode.hpp"
#include "../Kernel/VirtualMemory.hpp"
#include "../Kernel/Syscall.hpp"
#include <cstdio>
int main(){Davis::UserMode::GdtState g;Davis::UserMode::BuildGdt(g,0x800000);if(!Davis::UserMode::Validate(g))return 1;Davis::Vm::AddressSpace a;Davis::Vm::Init(a);if(!Davis::Vm::AddRegion(a,0x40000000,0x1000000,2,Davis::Vm::NoExecute))return 2;if(Davis::Vm::AddRegion(a,0x40001000,0x2000000,1,0))return 3;if(!Davis::Vm::Validate(a))return 4;Davis::Syscall::Frame f{Davis::Syscall::Yield};auto r=Davis::Syscall::Dispatch(f);if(r.error)return 5;std::puts("PASS user-mode foundations");}
