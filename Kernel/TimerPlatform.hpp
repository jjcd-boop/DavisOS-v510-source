#pragma once
#include "../Base/Types.hpp"
#include "ApicPlatform.hpp"
#include "LocalApicTimer.hpp"
#include "TimerCalibration.hpp"
#include "Idt.hpp"
namespace Davis::TimerPlatform {
enum class Phase:u8 { Cold, ApicVerified, GateInstalled, Calibrated, Armed, Failed };
struct State { Phase phase; ApicPlatform::State apic; TimerCalibration::Result calibration; u8 vector; u32 targetHz; bool interruptsEnabled; };
void Init(State&,u32 targetHz=250,u8 vector=LocalApicTimer::DefaultVector);
bool AcceptApic(State&,const ApicPlatform::State&);
bool InstallGate(State&,Idt::Table&,u16 kernelCode,void(*entry)());
bool AcceptCalibration(State&,const TimerCalibration::Result&);
bool ReadyToArm(const State&);
}
