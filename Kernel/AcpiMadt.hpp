#pragma once
#include "AcpiTables.hpp"
namespace Davis::AcpiMadt {
constexpr u32 MaxCpus=64;
constexpr u32 MaxIoApics=8;
constexpr u32 MaxOverrides=24;
struct Cpu { u32 apicId; u32 acpiUid; bool enabled; bool onlineCapable; bool x2Apic; };
struct IoApic { u8 id; u32 address; u32 gsiBase; };
struct Iso { u8 bus; u8 sourceIrq; u32 gsi; u16 flags; };
struct State {
 bool present,valid,pcAtCompatible;
 u32 localApicAddress;
 u32 cpuCount,enabledCpuCount,ioApicCount,overrideCount;
 Cpu cpus[MaxCpus]; IoApic ioApics[MaxIoApics]; Iso overrides[MaxOverrides];
};
bool Parse(const Acpi::State&,State&);
bool ResolveIsaIrq(const State&,u8 irq,u32&gsi,u16&flags);
}
