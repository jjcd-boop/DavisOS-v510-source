#define DAVIS_HOST_TEST 1
#include "../Kernel/Idt.hpp"
#include "../Kernel/UserMode.hpp"
#include <cstdio>
static void fake(){}
int main(){Davis::UserMode::GdtState g{};Davis::UserMode::BuildGdt(g,0x100000);Davis::Idt::Table t{};Davis::Idt::Init(t);Davis::Idt::SetInterruptGate(t,0x80,fake,g.selectors.kernelCode,3);if(!Davis::Idt::ValidateSyscallGate(t,0x80,g.selectors.kernelCode))return 1;auto &x=t.gate[0x80];if(((x.typeAttr>>5)&3)!=3)return 2;std::puts("PASS v0.60 IDT/syscall-gate model");}
