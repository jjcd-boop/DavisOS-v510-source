#pragma once
#include "../Base/Types.hpp"
namespace Davis::NetworkService { struct State{u64 pumps,pingReplies;u32 ethernetControllers,wifiControllers;u16 ethernetVendor,ethernetDevice,wifiVendor,wifiDevice;bool ready,nicReady,dhcpBound,gatewayResolved,dnsResolved;}; extern State state; void Init(); void PumpOnce(); }
