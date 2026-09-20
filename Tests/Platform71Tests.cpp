#define DAVIS_HOST_TEST 1
#include "../Kernel/TimeReference.hpp"
#include "../Kernel/LapicCalibration.hpp"
#include <cstdio>
int main(){using namespace Davis;auto t=TimeReference::Decode(0x16,1,100,24000000,2400,true);if(!t.usable||t.tscHz!=2400000000ull)return 1;auto f=TimeReference::Decode(0x16,0,0,0,3200,true);if(!f.usable||f.tscHz!=3200000000ull)return 2;auto bad=TimeReference::Decode(0x16,1,100,24000000,2400,false);if(bad.usable)return 3;auto c=LapicCalibration::FromSample(0xffffffffu,0xffe17b7fu,10000,250);if(!c.valid||!c.timer.initialCount)return 4;std::puts("PASS v0.71 TSC reference/LAPIC calibration model");return 0;}
