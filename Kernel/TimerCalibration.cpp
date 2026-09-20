#include "TimerCalibration.hpp"
namespace Davis::TimerCalibration {
Result FromMeasuredTicks(u64 ticks,u64 us,u32 target){Result r{};r.targetHz=target;if(!ticks||!us||!target)return r;r.busHz=(ticks*1000000ull)/us;u64 c=r.busHz/target;if(!c||c>0xffffffffull)return r;r.initialCount=(u32)c;r.valid=true;return r;}
}
