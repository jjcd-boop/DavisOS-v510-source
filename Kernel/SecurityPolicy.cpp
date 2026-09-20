#include "SecurityPolicy.hpp"
#include "Dxe2.hpp"
namespace Davis::SecurityPolicy {
bool ValidDomain(SecurityDomain::Kind d){return d==SecurityDomain::Kind::Kernel||d==SecurityDomain::Kind::SystemService||d==SecurityDomain::Kind::Driver||d==SecurityDomain::Kind::Application;}
bool CanUseRawHardware(SecurityDomain::Kind d){return d==SecurityDomain::Kind::Driver;}
bool CanRegisterSystemService(SecurityDomain::Kind d){return d==SecurityDomain::Kind::SystemService||d==SecurityDomain::Kind::Driver;}
u64 GrantFor(SecurityDomain::Kind d,u64 r){
 r&=Dxe2::KnownCapabilities;
 // Self-declared executable metadata never grants driver/service authority.
 // Trusted launchers choose the domain; policy chooses the usable subset.
 switch(d){
  case SecurityDomain::Kind::Application:return r&(Dxe2::CapWindow|Dxe2::CapFileRead|Dxe2::CapFileWrite|Dxe2::CapNetwork|Dxe2::CapIpc);
  case SecurityDomain::Kind::SystemService:return r&(Dxe2::CapWindow|Dxe2::CapFileRead|Dxe2::CapFileWrite|Dxe2::CapNetwork|Dxe2::CapIpc|Dxe2::CapServiceHost);
  case SecurityDomain::Kind::Driver:return r&(Dxe2::CapIpc|Dxe2::CapDriverIo|Dxe2::CapIrq|Dxe2::CapDma);
  case SecurityDomain::Kind::Kernel:return 0;
 }
 return 0;
}
}
