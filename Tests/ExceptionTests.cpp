#define DAVIS_HOST_TEST 1
#include "../Kernel/Exceptions.hpp"
#include "../Kernel/Idt.hpp"
#include <cstdio>
extern "C" void DavisExceptionUD(){} extern "C" void DavisExceptionGP(){} extern "C" void DavisExceptionPF(){}
int main(){using namespace Davis;Process::Image p{};p.state=Process::State::Running;Exceptions::SetCurrent(&p);Exceptions::Frame f{};f.cs=0x23;f.vector=14;f.error=5;f.rip=0x400123;Exceptions::DavisExceptionDispatch(&f);if(p.state!=Process::State::Faulted||!Exceptions::state.userFault||Exceptions::state.fatalKernelFault)return 1;Idt::Table t{};Idt::Init(t);Exceptions::InstallCoreGates(t,0x08);if((t.gate[14].typeAttr>>5&3)!=0)return 2;std::puts("PASS v0.61 exception containment model");}
