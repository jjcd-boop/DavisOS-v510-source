#pragma once
#include "../Base/Types.hpp"
namespace Davis::ApicPlatform {
constexpr u32 ApicFeatureBit = 1u<<9;
constexpr u32 ApicBaseMsr = 0x1B;
struct State { bool cpuidSupported; bool localApicPresent; bool msrEnabled; bool x2Apic; u64 physicalBase; bool usable; };
State Decode(u32 cpuidEdx,u64 apicBaseMsr);
State Probe();
}
