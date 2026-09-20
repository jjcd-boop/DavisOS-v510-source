#pragma once
#include "../Base/Types.hpp"
namespace Davis::KernelSecurity {
struct State { bool cpuid; bool nxSupported; bool nxEnabled; bool writeProtectEnabled; bool smepSupported; bool smapSupported; bool ready; u32 maxBasicLeaf; u32 maxExtendedLeaf; };
extern State state;
void Init();
[[noreturn]] void FatalIntegrityFault();
}
