#include "InterruptAudit.hpp"
namespace Davis::InterruptAudit {
void Init(State&s){s={};s.result=Result::Cold;}
bool Verify(State&s,const Idt::Table&id,u16 kc,const ApicPlatform::State&a,const InterruptController::State&ic,const LocalApicTimer::State&t,const TimerPlatform::State&p){
 Init(s);s.vector=p.vector;
 if(s.vector<0x40){s.result=Result::BadVector;return false;}
 const auto&g=id.gate[s.vector];s.gatePresent=(g.typeAttr&0x80)!=0;s.kernelOnly=((g.typeAttr>>5)&3)==0;
 if(!id.ready||!s.gatePresent||!s.kernelOnly||g.selector!=kc||(g.typeAttr&0x0f)!=0x0e){s.result=Result::BadGate;return false;}
 s.backendMatched=a.x2Apic?(ic.kind==InterruptController::Kind::X2Apic&&t.backend==LocalApicTimer::Backend::X2ApicMsr):(ic.kind==InterruptController::Kind::LocalApic&&t.backend==LocalApicTimer::Backend::XApicMmio);
 if(!s.backendMatched||!ic.ready){s.result=Result::BackendMismatch;return false;}
 s.timerMatched=t.configured&&!t.armed&&t.vector==p.vector&&p.calibration.valid&&p.calibration.initialCount==t.ticksPerQuantum;
 if(!s.timerMatched){s.result=Result::TimerMismatch;return false;}
 s.result=Result::Ready;return true;
}
}
