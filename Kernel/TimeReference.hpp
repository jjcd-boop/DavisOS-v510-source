#pragma once
#include "../Base/Types.hpp"
namespace Davis::TimeReference {
enum class Source:u8 { None, Cpuid15Tsc, Cpuid16Tsc };
struct State { Source source; u64 tscHz; bool invariant; bool usable; };
State Decode(u32 maxLeaf,u32 denom,u32 numer,u32 crystalHz,u32 baseMHz,bool invariant);
State Probe();
u64 ReadTsc();
bool WaitMicroseconds(const State&,u64 us);
}
