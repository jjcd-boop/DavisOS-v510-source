#include "TimerPlatform.hpp"
namespace Davis::TimerPlatform {
void Init(State&s,u32 hz,u8 vector){s={};s.phase=Phase::Cold;s.targetHz=hz;s.vector=vector;}
bool AcceptApic(State&s,const ApicPlatform::State&a){s.apic=a;if(!a.usable){s.phase=Phase::Failed;return false;}s.phase=Phase::ApicVerified;return true;}
bool InstallGate(State&s,Idt::Table&t,u16 kc,void(*entry)()){if(s.phase!=Phase::ApicVerified||!entry||kc==0){s.phase=Phase::Failed;return false;}Idt::SetInterruptGate(t,s.vector,entry,kc,0);auto&g=t.gate[s.vector];bool ok=(g.typeAttr&0x80)&&((g.typeAttr>>5)&3)==0&&g.selector==kc;if(!ok){s.phase=Phase::Failed;return false;}s.phase=Phase::GateInstalled;return true;}
bool AcceptCalibration(State&s,const TimerCalibration::Result&r){if(s.phase!=Phase::GateInstalled||!r.valid||r.targetHz!=s.targetHz||!r.initialCount){s.phase=Phase::Failed;return false;}s.calibration=r;s.phase=Phase::Calibrated;return true;}
bool ReadyToArm(const State&s){return s.phase==Phase::Calibrated&&s.apic.usable&&s.calibration.valid&&!s.interruptsEnabled;}
}
