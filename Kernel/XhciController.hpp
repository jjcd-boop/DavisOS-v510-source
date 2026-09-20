#pragma once
#include "XhciDma.hpp"
namespace Davis::Xhci {
enum class StartResult:u32 { Ok,InvalidState,FirmwareHandoffFailed,ControllerNotHalted,ResetTimeout,NotReady,DmaInvalid,RegisterReadbackFailed,PciEnableFailed,RunTimeout };
struct ControllerState { bool handoff,halted,reset,cnrClear,dmaValid,pciEnabled,dcbaaOk,crcrOk,erstOk,programmed,runSet,running; u32 lastUsbSts,lastUsbCmd,pageSize,pciCommand; u64 crcrExpected,crcrReadback; StartResult result; };
void InitController(ControllerState&);
StartResult StartController(State&,DmaState&,ControllerState&,u32 spinLimit=2000000);
}
