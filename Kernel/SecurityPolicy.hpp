#pragma once
#include "../Base/Types.hpp"
#include "SecurityDomain.hpp"
namespace Davis::SecurityPolicy {
// Capabilities that may be requested by ordinary DXE2 applications.
u64 GrantFor(SecurityDomain::Kind domain,u64 requested);
bool CanUseRawHardware(SecurityDomain::Kind domain);
bool CanRegisterSystemService(SecurityDomain::Kind domain);
bool ValidDomain(SecurityDomain::Kind domain);
}
