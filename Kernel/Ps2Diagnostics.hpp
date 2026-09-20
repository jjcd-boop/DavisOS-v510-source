#pragma once
#include "../Base/Types.hpp"
namespace Davis::Ps2Diagnostics {
struct State{bool probed,controllerPresent,port1Tested,port1Ok,keyboardPortEnabled,keyboardAck,keyboardBat,scanningEnabled,auxPortEnabled,auxAck,auxReset,auxBat,auxScanning;u8 initialStatus,configByte,keyboardTest,lastResponse,lastScan,lastAux;u64 keyboardBytes,auxBytes,polls,obfSeen;u8 liveStatus;};
extern State state;
void Init(); void NoteByte(u8 status,u8 value);
}
