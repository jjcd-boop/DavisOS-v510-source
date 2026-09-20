#pragma once
#include "TimerCalibration.hpp"
#include "TimeReference.hpp"
#include "LocalApicTimer.hpp"
namespace Davis::LapicCalibration {
struct Result { TimerCalibration::Result timer; u64 sampleUs; u32 startCount; u32 endCount; bool valid; };
Result FromSample(u32 start,u32 end,u64 us,u32 hz);
Result Measure(volatile u32* apic,const TimeReference::State&,u64 us,u32 hz,u8 div);
Result MeasureX2(const TimeReference::State&,u64 us,u32 hz,u8 div);
}
