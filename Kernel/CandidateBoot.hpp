#pragma once
#include "BootInfo.hpp"
namespace Davis::CandidateBoot {
enum class Checkpoint:u32 { KernelEntry=1, MemoryReady=2, RuntimeReady=3, HardwareReady=4, InputReady=5, DesktopReady=6, Ring3Ready=7, Healthy=8 };
void Mark(const BootInfo&,Checkpoint);
void AcknowledgeHealthy(const BootInfo&);
}
