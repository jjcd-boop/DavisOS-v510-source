#pragma once
#include "NetworkCore.hpp"
namespace Davis::Dhcp {
enum class Phase:u8{Idle,Discovering,Requesting,AwaitingAck,Bound,Failed};
struct State{Phase phase;u32 xid;u32 ticks,retries;Net::Ipv4 offered,server;u64 discovers,offers,requests,acks;};extern State state;
void Init();bool BuildDiscover(Net::Packet&);bool BuildRequest(Net::Packet&);void OnFrame(const u8*,u32);void Tick();
}
