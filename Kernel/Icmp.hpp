#pragma once
#include "NetworkCore.hpp"
namespace Davis::Icmp {struct State{u16 sequence;u64 sent,replies;};extern State state;void Init();bool BuildEcho(Net::Ipv4,Net::Mac,Net::Packet&);void OnFrame(const u8*,u32);}
