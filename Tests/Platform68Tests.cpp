#define DAVIS_HOST_TEST 1
#include "../Kernel/TimerFrame.hpp"
#include "../Kernel/InterruptController.hpp"
#include <cstdio>
int main(){
 Davis::CpuContext::Frame k{}; k.rip=0x2000000;k.cs=0x08;
 auto kv=Davis::TimerFrame::Normalize(k); if(kv.safeToPreempt||kv.hasHardwareRspSs)return 1;
 Davis::CpuContext::Frame u{};u.rip=0x10000000000ull;u.cs=0x23;u.rsp=0x10001000000ull;u.ss=0x1b;
 auto uv=Davis::TimerFrame::Normalize(u);if(!uv.safeToPreempt||!uv.hasHardwareRspSs)return 2;
 Davis::InterruptController::State ic{};Davis::InterruptController::InitNone(ic);Davis::InterruptController::EndOfInterrupt(ic);if(ic.eoiCount)return 3;
 if(Davis::InterruptController::InitLocalApic(ic,0xfee00001ull))return 4;
 std::puts("PASS v0.68 normalized timer-frame/EOI model");return 0;
}
