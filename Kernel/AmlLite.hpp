#pragma once
#include "../Base/Types.hpp"
#include "AcpiTables.hpp"
namespace Davis::AmlLite {
struct HidDevice { char hid[16]; bool hasCrs; bool likelyTouchpad; };
struct State { bool dsdtScanned; u32 deviceCount; u32 hidCount; u32 touchpadCandidates; HidDevice devices[16]; };
void Scan(const Acpi::State&, State&);
}
