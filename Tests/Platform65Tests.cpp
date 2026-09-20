#define DAVIS_HOST_TEST
#include "../Kernel/UserMode.hpp"
#include "../Kernel/PlatformState.hpp"
#include <cstdio>
int main(){Davis::UserMode::GdtState g{};Davis::UserMode::BuildGdt(g,0x400000);if(!Davis::UserMode::Validate(g))return 1;auto s=Davis::PlatformState::Capture(g);if(!Davis::PlatformState::ValidateOwned(s,g))return 2;if(s.cs!=0x08||s.ss!=0x10||s.tr!=0x28)return 3;std::puts("PASS v0.65 owned GDT/TSS/platform-state model");return 0;}
