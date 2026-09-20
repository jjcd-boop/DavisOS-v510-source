#include "SmpLapicPlatform.hpp"
namespace Davis::SmpLapicPlatform {
bool Init(State&s,const AcpiMadt::State&m,const ApicPlatform::State&a,const LapicHardware::Access&access){
 s={};
 if(!m.valid||!m.enabledCpuCount){s.failure=Failure::MadtInvalid;return false;}
 if(!a.usable){s.failure=Failure::ApicUnavailable;return false;}
 bool wide=false;u32 enabled=0;
 for(u32 i=0;i<m.cpuCount;i++)if(m.cpus[i].enabled){enabled++;if(m.cpus[i].apicId>255||m.cpus[i].x2Apic)wide=true;}
 if(!enabled){s.failure=Failure::MadtInvalid;return false;}
 // xAPIC uses the memory-mapped LAPIC base. Firmware MADT and IA32_APIC_BASE must
 // describe the same page; silently choosing one would target the wrong device.
 if(!a.x2Apic){
  if(wide){s.failure=Failure::WideIdRequiresX2Apic;return false;}
  if(!m.localApicAddress||a.physicalBase!=(u64)m.localApicAddress){s.failure=Failure::AddressMismatch;return false;}
  s.mode=LapicIpi::Mode::XApic;s.base=a.physicalBase;
 }else{
  s.mode=LapicIpi::Mode::X2Apic;s.base=a.physicalBase;
 }
 if(!LapicHardware::Init(s.hardware,s.mode,s.base,access)){s.failure=Failure::AccessInvalid;return false;}
 s.transport=LapicHardware::MakeTransport(s.hardware);s.enabledCpus=enabled;s.failure=Failure::None;s.ready=true;return true;
}
}
