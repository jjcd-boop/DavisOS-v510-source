#include "Kernel/SmpLapicPlatform.hpp"
#include <cstdio>
using namespace Davis;
struct F{u32 lo=0,hi=0;u64 msr=0;};
static bool rm(void*v,u64 a,u32&o){auto*f=(F*)v;if((a&0xfff)!=0x300)return false;o=f->lo&~LapicHardware::DeliveryStatusBit;return true;}
static bool wm(void*v,u64 a,u32 x){auto*f=(F*)v;if((a&0xfff)==0x300)f->lo=x;else if((a&0xfff)==0x310)f->hi=x;else return false;return true;}
static bool rr(void*v,u32 m,u64&o){if(m!=0x830)return false;o=((F*)v)->msr&~(u64)LapicHardware::DeliveryStatusBit;return true;}
static bool wr(void*v,u32 m,u64 x){if(m!=0x830)return false;((F*)v)->msr=x;return true;}
static void du(void*,u32){}
static AcpiMadt::State madt(bool wide){AcpiMadt::State m{};m.valid=true;m.localApicAddress=0xfee00000;m.cpuCount=4;m.enabledCpuCount=4;for(u32 i=0;i<4;i++){m.cpus[i].enabled=true;m.cpus[i].apicId=i;}if(wide){m.cpus[3].apicId=0x1234;m.cpus[3].x2Apic=true;}return m;}
static ApicPlatform::State apic(bool x2){ApicPlatform::State a{};a.cpuidSupported=a.localApicPresent=a.msrEnabled=a.usable=true;a.x2Apic=x2;a.physicalBase=0xfee00000;return a;}
int main(){u64 cycles=0;for(u32 i=0;i<100000;i++){F f{};LapicHardware::Access x{rm,wm,rr,wr,du,&f,32};SmpLapicPlatform::State s{};bool x2=i&1;auto m=madt(x2);auto a=apic(x2);if(!SmpLapicPlatform::Init(s,m,a,x)||!s.ready||s.enabledCpus!=4)return 1;ApStartup::Plan p{};if(!ApStartup::BuildPlan(x2?0x1234:3,0x8000,p))return 2;LapicIpi::Stats st{};if(!LapicIpi::ExecutePlan(s.transport,p,st)||st.sent!=4)return 3;cycles++;}
F f{};LapicHardware::Access x{rm,wm,rr,wr,du,&f,32};SmpLapicPlatform::State s{};auto wide=madt(true);auto xa=apic(false);if(SmpLapicPlatform::Init(s,wide,xa,x)||s.failure!=SmpLapicPlatform::Failure::WideIdRequiresX2Apic)return 4;auto m=madt(false);xa.physicalBase=0xfee01000;if(SmpLapicPlatform::Init(s,m,xa,x)||s.failure!=SmpLapicPlatform::Failure::AddressMismatch)return 5;auto bad=apic(false);bad.usable=false;if(SmpLapicPlatform::Init(s,m,bad,x)||s.failure!=SmpLapicPlatform::Failure::ApicUnavailable)return 6;
std::printf("PASS smp_lapic_platform cycles=%llu xapic=1 x2apic=1 wide_id_reject=1 base_mismatch_reject=1\n",(unsigned long long)cycles);}
