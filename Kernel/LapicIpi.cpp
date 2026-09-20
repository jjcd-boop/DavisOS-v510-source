#include "LapicIpi.hpp"
namespace Davis::LapicIpi {
bool Validate(const Transport&t){if(!t.writeIcr||!t.delayUs)return false;if(t.mode==Mode::XApic&&(!t.lapicBase||(t.lapicBase&0xfffull)))return false;return true;}
bool SendStep(const Transport&t,const ApStartup::Step&s,Stats&st){if(!Validate(t))return false;if(t.mode==Mode::XApic&&s.apicId>255)return false;u64 icr=t.mode==Mode::X2Apic?ApStartup::EncodeX2ApicIcr(s.kind,s.apicId,s.vector):ApStartup::EncodeXApicIcr(s.kind,s.apicId,s.vector);if(!t.writeIcr(t.context,icr))return false;st.sent++;if(s.kind==ApStartup::Kind::InitAssert||s.kind==ApStartup::Kind::InitDeassert)st.init++;else st.sipi++;if(s.delayUs){t.delayUs(t.context,s.delayUs);st.delayedUs+=s.delayUs;}return true;}
bool ExecutePlan(const Transport&t,const ApStartup::Plan&p,Stats&st){st={};if(!p.valid||p.count==0||p.count>ApStartup::MaxSequenceSteps||!ApStartup::ValidateTrampoline(p.trampolinePhysical))return false;for(u32 i=0;i<p.count;i++)if(!SendStep(t,p.steps[i],st))return false;return st.sent==p.count;}
}
