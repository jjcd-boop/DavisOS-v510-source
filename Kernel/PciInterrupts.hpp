#pragma once
#include "../Base/Types.hpp"
#include "Idt.hpp"
namespace Davis { namespace Process { struct Image; } namespace DeviceResourceBroker { struct Resource; } }
namespace Davis::PciInterrupts {
static constexpr u8 FirstVector=0x50, LastVector=0x5f;
void InstallGates(Idt::Table&,u16 kernelCode);
bool ConfigureMsi(const Process::Image&,DeviceResourceBroker::Resource&,u8* vectorOut);
void ReleaseProcess(u64 processId);
extern "C" void DavisExternalIrqDispatch(u64 vector);
}
