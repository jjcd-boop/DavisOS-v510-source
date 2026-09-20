#pragma once
#include "../Base/Types.hpp"
namespace Davis::LocalApicTimer {
constexpr u8 DefaultVector=0x40;
enum class Mode:u8 { Disabled, OneShot, Periodic };
enum class Backend:u8 { None, XApicMmio, X2ApicMsr };
struct State { volatile u32* lapic; u32 ticksPerQuantum; u8 vector; u8 divideCode; Mode mode; Backend backend; bool configured; bool armed; u64 interrupts; };
void Init(State&);
bool ConfigurePeriodic(State&,u64 lapicBase,u8 vector,u32 initialCount,u8 divideCode=0x3);
bool ConfigurePeriodicX2(State&,u8 vector,u32 initialCount,u8 divideCode=0x3);
void Arm(State&); void Stop(State&); void NoteInterrupt(State&); u32 CurrentCount(const State&); bool Validate(const State&);
}
