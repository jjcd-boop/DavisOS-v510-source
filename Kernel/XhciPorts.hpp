#pragma once
#include "Xhci.hpp"
#include "XhciController.hpp"
namespace Davis::Xhci {
enum class PortResult:u32{Ok,InvalidState,NoConnectedPort,ResetTimeout,EnableTimeout};
struct PortInfo{u8 port;bool connected,enabled,powered;u8 speed;u32 portsc;};
struct PortState{PortInfo ports[32];u32 count;u8 selectedPort;PortResult result;};
void ScanPorts(const State&,const ControllerState&,PortState&);
PortResult ResetPort(const State&,const ControllerState&,u8 port,PortState&,u32 spinLimit=2000000);
PortResult ResetFirstConnectedPort(const State&,const ControllerState&,PortState&,u32 spinLimit=2000000);
}
