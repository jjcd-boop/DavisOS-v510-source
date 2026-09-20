#pragma once
#include "../Base/Types.hpp"
#include "ApStartup.hpp"
namespace Davis::LapicIpi {
enum class Mode:u8 { XApic, X2Apic };
struct Transport { Mode mode; u64 lapicBase; bool (*writeIcr)(void*,u64); void (*delayUs)(void*,u32); void* context; };
struct Stats { u32 sent; u32 init; u32 sipi; u64 delayedUs; };
bool Validate(const Transport&);
bool SendStep(const Transport&,const ApStartup::Step&,Stats&);
bool ExecutePlan(const Transport&,const ApStartup::Plan&,Stats&);
}
