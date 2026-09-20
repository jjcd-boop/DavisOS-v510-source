#define DAVIS_HOST_TEST 1
#include "../Kernel/LocalApicTimer.hpp"
#include "../Kernel/TimerCalibration.hpp"
#include <cstdio>
int main(){Davis::LocalApicTimer::State s;Davis::LocalApicTimer::Init(s);if(s.configured)return 1;auto r=Davis::TimerCalibration::FromMeasuredTicks(1000000,10000,250);if(!r.valid||r.busHz!=100000000ull||r.initialCount!=400000)return 2;if(Davis::LocalApicTimer::ConfigurePeriodic(s,0xfee00001,0x20,r.initialCount))return 3;
Davis::LocalApicTimer::Init(s);if(Davis::LocalApicTimer::ConfigurePeriodic(s,0xfee00000,0x1f,100))return 4;if(!Davis::LocalApicTimer::ConfigurePeriodic(s,0xfee00000,0x20,400000))return 5;if(!Davis::LocalApicTimer::Validate(s))return 6;std::puts("PASS v0.69 LAPIC timer/calibration model");return 0;}
