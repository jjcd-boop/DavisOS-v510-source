#define DAVIS_HOST_TEST 1
#include "../Kernel/ApicPlatform.hpp"
#include "../Kernel/TimerPlatform.hpp"
#include <cstdio>
static void timerEntry(){}
int main(){using namespace Davis;auto a=ApicPlatform::Decode(1u<<9,0xfee00000ull|(1ull<<11));if(!a.usable||a.physicalBase!=0xfee00000ull)return 1;auto x2=ApicPlatform::Decode(1u<<9,0xfee00000ull|(1ull<<11)|(1ull<<10));if(x2.usable)return 2;Idt::Table idt{};Idt::Init(idt);TimerPlatform::State s{};TimerPlatform::Init(s);if(!TimerPlatform::AcceptApic(s,a))return 3;if(!TimerPlatform::InstallGate(s,idt,0x08,timerEntry))return 4;auto r=TimerCalibration::FromMeasuredTicks(1000000,10000,250);if(!TimerPlatform::AcceptCalibration(s,r)||!TimerPlatform::ReadyToArm(s))return 5;std::puts("PASS v0.70 APIC discovery/timer activation gate model");return 0;}
