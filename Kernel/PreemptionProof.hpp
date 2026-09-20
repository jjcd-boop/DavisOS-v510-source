#pragma once
#include "../Base/Types.hpp"
namespace Davis::PreemptionProof {
enum class Fault:u32 { None, MissingProcess, SameProcess, SameCr3, SameRsp0, UnalignedCr3, UnalignedRsp0, MissingKernelContract, WrongSequence, FaultDidNotIsolate };
struct Endpoint { u64 processId; u64 cr3; u64 kernelRsp0; bool sharedKernelMappings; };
struct State { Endpoint a,b; u64 observedSwitches; u64 lastProcessId; u64 faultedProcessId; Fault fault; bool armed; bool passed; };
bool Arm(State&,const Endpoint&,const Endpoint&);
bool Observe(State&,u64 processId,u64 activeCr3,u64 activeRsp0);
bool ObserveFaultIsolation(State&,u64 faultedProcessId,u64 survivingProcessId);
void Fail(State&,Fault);
}
