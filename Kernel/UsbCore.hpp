#pragma once
#include "../Base/Types.hpp"
namespace Davis::Usb {
static constexpr u32 MaxDevices=32, MaxInterfaces=8, MaxEndpoints=16;
enum class Speed:u8 { Unknown, Low, Full, High, Super, SuperPlus };
enum class DeviceState:u8 { Empty, Attached, Addressed, Configured, Removed, Faulted };
struct Endpoint { u8 address,attributes; u16 maxPacket; u8 interval; bool valid; };
struct Interface { u8 number,alternate,klass,subclass,protocol,endpointCount; Endpoint endpoints[MaxEndpoints]; bool valid; };
struct Device { u64 generation; u8 slotId,port,address,configuration; u16 vendorId,productId,bcdUsb; Speed speed; DeviceState state; u8 interfaceCount; Interface interfaces[MaxInterfaces]; };
struct Registry { Device devices[MaxDevices]; u64 generation; u64 attaches,removes,descriptorErrors; };
void Init(Registry&); Device* Attach(Registry&,u8 port,Speed speed); void RemovePort(Registry&,u8 port); Device* FindPort(Registry&,u8 port);
bool ParseDeviceDescriptor(Device&,const u8*,usize); bool ParseConfiguration(Device&,const u8*,usize);
}
