#pragma once
#include "BootInfo.hpp"
#include "Graphics.hpp"
#include "Pci.hpp"
namespace Davis::HardwareProbe {
struct State { Graphics::Surface screen; Pci::State pci; u32 row; bool enabled; };
extern State state;
void Begin(const BootInfo&);
void ScanAndDisplay(const BootInfo&);
[[noreturn]] void CompleteAndHalt();
}
