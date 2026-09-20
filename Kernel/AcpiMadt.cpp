#include "AcpiMadt.hpp"
namespace Davis::AcpiMadt {
static u16 rd16(const u8*p){return (u16)p[0]|((u16)p[1]<<8);} static u32 rd32(const u8*p){return (u32)p[0]|((u32)p[1]<<8)|((u32)p[2]<<16)|((u32)p[3]<<24);}
bool Parse(const Acpi::State&a,State&s){s={};auto*t=Acpi::Find(a,"APIC");if(!t||!t->checksumOk||!t->address||t->length<44)return false;s.present=true;const u8*b=(const u8*)t->address;s.localApicAddress=rd32(b+36);u32 f=rd32(b+40);s.pcAtCompatible=(f&1u)!=0;const u8*p=b+44,*e=b+t->length;while(p<e){if((usize)(e-p)<2){return false;}u8 type=p[0],len=p[1];if(len<2||(usize)(e-p)<len)return false;
 if(type==0&&len>=8){u32 flags=rd32(p+4);if(s.cpuCount<MaxCpus){auto&c=s.cpus[s.cpuCount++];c.acpiUid=p[2];c.apicId=p[3];c.enabled=(flags&1u)!=0;c.onlineCapable=(flags&2u)!=0;c.x2Apic=false;if(c.enabled)s.enabledCpuCount++;}}
 else if(type==1&&len>=12){if(s.ioApicCount<MaxIoApics){auto&i=s.ioApics[s.ioApicCount++];i.id=p[2];i.address=rd32(p+4);i.gsiBase=rd32(p+8);}}
 else if(type==2&&len>=10){if(s.overrideCount<MaxOverrides){auto&o=s.overrides[s.overrideCount++];o.bus=p[2];o.sourceIrq=p[3];o.gsi=rd32(p+4);o.flags=rd16(p+8);}}
 else if(type==5&&len>=12){u64 x=(u64)rd32(p+4)|((u64)rd32(p+8)<<32);if(x&&!(x&0xfffull)&&x<=0xffffffffull)s.localApicAddress=(u32)x;}
 else if(type==9&&len>=16){u32 flags=rd32(p+8);if(s.cpuCount<MaxCpus){auto&c=s.cpus[s.cpuCount++];c.apicId=rd32(p+4);c.acpiUid=rd32(p+12);c.enabled=(flags&1u)!=0;c.onlineCapable=(flags&2u)!=0;c.x2Apic=true;if(c.enabled)s.enabledCpuCount++;}}
 p+=len;}s.valid=s.cpuCount>0&&s.enabledCpuCount>0&&s.localApicAddress!=0;return s.valid;}
bool ResolveIsaIrq(const State&s,u8 irq,u32&gsi,u16&flags){if(!s.valid)return false;gsi=irq;flags=0;for(u32 i=0;i<s.overrideCount;i++){const auto&o=s.overrides[i];if(o.bus==0&&o.sourceIrq==irq){gsi=o.gsi;flags=o.flags;break;}}return true;}
}
