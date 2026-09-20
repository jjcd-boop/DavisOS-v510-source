#include "XhciCommands.hpp"
namespace Davis::Xhci {
static constexpr u32 RingTrbs=256, TypeShift=10, EnableSlotType=9, CommandCompletionType=33;
static inline void Barrier(){__asm__ __volatile__("" ::: "memory");}
void InitCommandRing(DmaState&d){d.commandIndex=0;d.eventIndex=0;d.commandCycle=true;d.eventCycle=true;}
bool QueueCommand(DmaState&d,u64 parameter,u32 status,u32 control,u64&phys){
 if(!d.valid||d.commandIndex>=RingTrbs-1)return false;
 Trb*t=CommandTrbs(d);u32 i=d.commandIndex;phys=d.commandRing.physical+(u64)i*sizeof(Trb);
 t[i].parameterLo=(u32)parameter;t[i].parameterHi=(u32)(parameter>>32);t[i].status=status;t[i].control=(control&~1u)|(d.commandCycle?1u:0u);Barrier();
 if(++d.commandIndex==RingTrbs-1){ // producer wraps through permanent Link TRB
  t[RingTrbs-1].control=(6u<<TypeShift)|2u|(d.commandCycle?1u:0u);Barrier();d.commandIndex=0;d.commandCycle=!d.commandCycle;
 }
 return true;
}
CommandResult RingDoorbell0(const State&s,const ControllerState&c){if(!s.capabilitiesValid||!c.running)return CommandResult::InvalidState;volatile u32*db=(volatile u32*)(uptr)(s.mmio+(s.dboff&~3u));*db=0;Barrier();return CommandResult::Ok;}
static void AdvanceEvent(const State&s,DmaState&d){if(++d.eventIndex==RingTrbs){d.eventIndex=0;d.eventCycle=!d.eventCycle;}volatile u64*erdp=(volatile u64*)(uptr)(s.mmio+(s.rtsoff&~0x1fu)+0x38);*erdp=(d.eventRing.physical+(u64)d.eventIndex*sizeof(Trb))|(1ull<<3);Barrier();}
bool PollCommandCompletion(const State&s,DmaState&d,u64 commandTrb,CommandCompletion&o,u32 spins){o={CommandResult::Timeout,0,0,commandTrb};if(!d.valid)return false;
 while(spins--){Trb*e=&EventTrbs(d)[d.eventIndex];u32 ctl=e->control;if((ctl&1u)!=(d.eventCycle?1u:0u)){__asm__ __volatile__("pause");continue;}u32 type=(ctl>>TypeShift)&0x3f;u64 ptr=(u64)e->parameterLo|((u64)e->parameterHi<<32);u8 cc=(u8)(e->status>>24),slot=(u8)(ctl>>24);AdvanceEvent(s,d);if(type!=CommandCompletionType)continue;if((ptr&~0xfull)!=(commandTrb&~0xfull))continue;o.completionCode=cc;o.slotId=slot;o.commandTrb=ptr;o.result=(cc==1)?CommandResult::Ok:CommandResult::EventError;return true;}return false;}
CommandResult EnableSlot(const State&s,const ControllerState&c,DmaState&d,CommandCompletion&o,u32 spins){if(!c.running||!d.valid)return CommandResult::InvalidState;u64 p=0;if(!QueueCommand(d,0,0,EnableSlotType<<TypeShift,p))return CommandResult::RingFull;auto r=RingDoorbell0(s,c);if(r!=CommandResult::Ok)return r;if(!PollCommandCompletion(s,d,p,o,spins))return CommandResult::Timeout;return o.result;}
}
