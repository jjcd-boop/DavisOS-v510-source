#include "PlatformProfile.hpp"
namespace Davis::PlatformProfile {
void Detect(const Pci::State&p,State&s){s={};s.usbXhci=p.xhciFound;s.nvme=p.nvmeFound;s.ahci=p.ahciFound;s.hda=p.hdaFound;s.network=p.networkFound;s.i2cCandidate=p.i2cFound;s.i2cCandidates=p.i2cCount;s.legacyInputFallback=true;
 for(u32 i=0;i<p.count;i++){const auto&d=p.devices[i];if(d.vendor==0x8086){s.cpuFamily=CpuFamily::Intel;if(d.classCode==0x03)s.integratedDisplay=true;if(d.classCode==0x0C&&d.subclass==0x80)s.intelSerialIo=true;}else if(d.vendor==0x1022){if(s.cpuFamily==CpuFamily::Unknown)s.cpuFamily=CpuFamily::Amd;if(d.classCode==0x03)s.amdDisplay=true;}else if(d.vendor==0x10DE&&d.classCode==0x03)s.nvidiaDisplay=true;}
 s.preferI2cHid=s.i2cCandidate; s.hybridGraphics=s.nvidiaDisplay&&(s.integratedDisplay||s.amdDisplay);
}
}
