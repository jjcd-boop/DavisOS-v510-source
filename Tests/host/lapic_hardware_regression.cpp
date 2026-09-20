#include "Kernel/LapicHardware.hpp"
#include <cstdio>
using namespace Davis;
struct Fake { u32 low=0,high=0; u64 msr=0; u64 mmioWrites=0,msrWrites=0,delays=0; u32 busy=0; bool stuck=false; };
static bool rm(void*v,u64 a,u32&o){auto*f=(Fake*)v;if((a&0xfff)!=LapicHardware::IcrLowOffset)return false;if(f->stuck){o=LapicHardware::DeliveryStatusBit;return true;}if(f->busy){f->busy--;o=LapicHardware::DeliveryStatusBit;}else o=f->low&~LapicHardware::DeliveryStatusBit;return true;}
static bool wm(void*v,u64 a,u32 x){auto*f=(Fake*)v;if((a&0xfff)==LapicHardware::IcrHighOffset)f->high=x;else if((a&0xfff)==LapicHardware::IcrLowOffset){f->low=x;f->busy=1;}else return false;f->mmioWrites++;return true;}
static bool rr(void*v,u32 m,u64&o){auto*f=(Fake*)v;if(m!=LapicHardware::X2ApicIcrMsr)return false;if(f->stuck){o=LapicHardware::DeliveryStatusBit;return true;}if(f->busy){f->busy--;o=f->msr|LapicHardware::DeliveryStatusBit;}else o=f->msr&~(u64)LapicHardware::DeliveryStatusBit;return true;}
static bool wr(void*v,u32 m,u64 x){auto*f=(Fake*)v;if(m!=LapicHardware::X2ApicIcrMsr)return false;f->msr=x;f->busy=1;f->msrWrites++;return true;}
static void du(void*v,u32 x){((Fake*)v)->delays+=x;}
int main(){u64 plans=0,steps=0;for(u32 z=0;z<100000;z++){Fake f{};LapicHardware::Access a{rm,wm,rr,wr,du,&f,32};LapicHardware::State s{};auto mode=(z&1)?LapicIpi::Mode::X2Apic:LapicIpi::Mode::XApic;if(!LapicHardware::Init(s,mode,0xfee00000,a))return 1;ApStartup::Plan p{};u32 id=mode==LapicIpi::Mode::X2Apic?0x1234u:(z&255u);if(!ApStartup::BuildPlan(id,0x8000,p))return 2;LapicIpi::Stats st{};auto t=LapicHardware::MakeTransport(s);if(!LapicIpi::ExecutePlan(t,p,st)||st.sent!=4||s.writes!=4)return 3;if(mode==LapicIpi::Mode::XApic){if(f.mmioWrites!=8||f.high!=(id<<24))return 4;}else if(f.msrWrites!=4)return 5;plans++;steps+=st.sent;}
Fake f{};f.stuck=true;LapicHardware::Access a{rm,wm,rr,wr,du,&f,3};LapicHardware::State s{};if(!LapicHardware::Init(s,LapicIpi::Mode::XApic,0xfee00000,a))return 6;auto t=LapicHardware::MakeTransport(s);ApStartup::Plan p{};ApStartup::BuildPlan(1,0x8000,p);LapicIpi::Stats st{};if(LapicIpi::ExecutePlan(t,p,st))return 7;
std::printf("PASS lapic_hardware plans=%llu steps=%llu timeout_reject=1 xapic=1 x2apic=1\n",(unsigned long long)plans,(unsigned long long)steps);}
