#pragma once
#include "XhciDma.hpp"
#include "XhciController.hpp"
namespace Davis::Xhci {
enum class CommandResult:u32 { Ok,InvalidState,RingFull,Timeout,EventError };
struct CommandCompletion { CommandResult result; u8 completionCode,slotId; u64 commandTrb; };
void InitCommandRing(DmaState&);
bool QueueCommand(DmaState&,u64 parameter,u32 status,u32 control,u64& trbPhysical);
CommandResult RingDoorbell0(const State&,const ControllerState&);
bool PollCommandCompletion(const State&,DmaState&,u64 commandTrb,CommandCompletion&,u32 spinLimit=2000000);
CommandResult EnableSlot(const State&,const ControllerState&,DmaState&,CommandCompletion&,u32 spinLimit=2000000);
}
