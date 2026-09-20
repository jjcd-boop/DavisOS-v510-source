#pragma once
#include "NetworkCore.hpp"
namespace Davis::Arp {
struct Entry{Net::Ipv4 ip;Net::Mac mac;u32 age;bool valid;};
struct State{Entry entries[16];u64 requests,replies,learned;}; extern State state;
void Init(); void Tick(); void OnFrame(const u8*,u32); bool Lookup(Net::Ipv4,Net::Mac&); bool Request(Net::Ipv4,Net::Packet&);
}
