#pragma once
#include "AcpiMadt.hpp"
#include "ApicPlatform.hpp"
#include "LapicHardware.hpp"
namespace Davis::SmpLapicPlatform {
enum class Failure:u8 { None, MadtInvalid, ApicUnavailable, AddressMismatch, WideIdRequiresX2Apic, AccessInvalid };
struct State { LapicHardware::State hardware; LapicIpi::Transport transport; LapicIpi::Mode mode; u64 base; u32 enabledCpus; Failure failure; bool ready; };
bool Init(State&,const AcpiMadt::State&,const ApicPlatform::State&,const LapicHardware::Access&);
}
