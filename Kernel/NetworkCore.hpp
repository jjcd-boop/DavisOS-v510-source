#pragma once
#include "../Base/Types.hpp"
namespace Davis::Net {
struct Mac { u8 b[6]; };
struct Ipv4 { u8 b[4]; };
static constexpr u32 MaxFrame=1536;
struct Packet { u8 data[MaxFrame]; u32 bytes; };
struct Config { Mac mac; Ipv4 address,netmask,gateway,dns; bool link,configured; };
struct State { Config config; u64 txPackets,rxPackets,drops; u16 nextIpId; u16 nextEphemeral; bool ready; };
struct RouteDecision { Ipv4 nextHop; bool onLink; bool valid; };
extern State state;
void Init();
u16 Checksum16(const void* data,u32 bytes);
bool IsZero(Ipv4); bool SameSubnet(Ipv4 a,Ipv4 b,Ipv4 mask); RouteDecision RouteTo(Ipv4 destination);
bool BuildArpRequest(Packet& out,Mac source,Ipv4 sourceIp,Ipv4 targetIp);
bool BuildIpv4Udp(Packet& out,Mac source,Mac destination,Ipv4 sourceIp,Ipv4 destinationIp,u16 sourcePort,u16 destinationPort,const void* payload,u32 payloadBytes);
bool ParseIpv4Udp(const u8* frame,u32 bytes,Ipv4& source,Ipv4& destination,u16& sourcePort,u16& destinationPort,const u8*& payload,u32& payloadBytes);
}
