#include "InterruptAddressSpace.hpp"
namespace Davis::InterruptAddressSpace {
Contract FromApic(const ApicPlatform::State&a){Contract c{};c.lapicBase=a.physicalBase;c.lapicMmioRequired=a.usable&&!a.x2Apic;return c;}
bool Install(Contract&c,Paging::Space&s,Memory::PageAllocator&a){
 if(!c.lapicMmioRequired){c.installed=true;return true;}
 c.installed=DeviceMappings::InstallSupervisorMmio(s,a,c.lapicBase,1)==DeviceMappings::Result::Ok;return c.installed;
}
bool Verify(const Contract&c,const Paging::Space&s){if(!c.installed)return false;if(!c.lapicMmioRequired)return true;return DeviceMappings::VerifySupervisorMmio(s,c.lapicBase,1);}
}
