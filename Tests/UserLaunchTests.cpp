#define DAVIS_HOST_TEST 1
#include "../Kernel/UserLaunch.hpp"
#include <cstdio>
int main(){Davis::UserMode::GdtState g{};Davis::UserMode::BuildGdt(g,0x90000);Davis::UserLaunch::Frame f{};if(!Davis::UserLaunch::Build(f,0x40000000,0x70010000,0x200000,g))return 1;if(f.cs!=0x23||f.ss!=0x1b||f.rflags!=0x202)return 2;std::puts("PASS user launch frame");}
