#pragma once
#include "../Base/Types.hpp"
namespace Davis::TimerCalibration {
struct Result { u64 busHz; u32 targetHz; u32 initialCount; bool valid; };
Result FromMeasuredTicks(u64 elapsedLapicTicks,u64 elapsedMicroseconds,u32 targetHz);
}
