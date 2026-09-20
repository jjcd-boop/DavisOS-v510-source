#define DAVIS_HOST_TEST 1
#include "../Kernel/TrapExit.hpp"
#include <cstdio>
int main(){Davis::Process::Image p{};p.state=Davis::Process::State::Running;Davis::TrapExit::Arm(&p);Davis::TrapExit::RequestExit(42);if(!Davis::TrapExit::ShouldAbandonUser()||p.state!=Davis::Process::State::Exited||p.exitCode!=42)return 1;Davis::TrapExit::Arm(&p);p.state=Davis::Process::State::Running;Davis::TrapExit::RequestFault(14);if(p.state!=Davis::Process::State::Faulted||p.exitCode!=-14)return 2;puts("PASS trap-exit state machine");}
