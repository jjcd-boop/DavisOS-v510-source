#pragma once
#include "../Base/Types.hpp"
namespace Davis::InterruptController {
enum class Kind:u8 { None, LocalApic, X2Apic };
struct State { Kind kind; volatile u32* lapic; u64 eoiCount; bool ready; };
void InitNone(State&);
bool InitLocalApic(State&,u64 mmioBase);
bool InitX2Apic(State&);
void EndOfInterrupt(State&);
void SetGlobal(State*);
extern "C" void DavisTimerEoi();
}
