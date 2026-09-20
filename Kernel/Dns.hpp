#pragma once
#include "NetworkCore.hpp"
namespace Davis::Dns { struct State{u16 nextId,pendingId;char pending[96];Net::Ipv4 answer;u64 queries,responses,timeouts;u32 waitTicks,retries;bool waiting,resolved,retryNeeded;};extern State state;void Init();bool BuildQuery(const char*,Net::Mac,Net::Packet&);void OnFrame(const u8*,u32);void Tick(); }
