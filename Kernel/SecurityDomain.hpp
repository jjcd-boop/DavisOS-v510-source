#pragma once
#include "../Base/Types.hpp"
namespace Davis::SecurityDomain {
enum class Kind:u8 { Kernel=0, SystemService=1, Driver=2, Application=3 };
// All non-kernel domains execute at CPL3. Kind is an authorization domain,
// never a substitute for CPU privilege separation.
constexpr bool IsUser(Kind k){return k!=Kind::Kernel;}
constexpr u8 CpuRing(Kind k){return k==Kind::Kernel?0:3;}
const char* Name(Kind);
}
