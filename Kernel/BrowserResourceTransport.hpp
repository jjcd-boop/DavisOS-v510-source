#pragma once
#include "BrowserResources.hpp"
#include "BrowserCore.hpp"
#include "Tcp.hpp"
namespace Davis::BrowserResourceTransport {
static constexpr u32 MaxBody=12288;
enum class Phase:u8{Idle,Resolving,Connecting,Sending,Receiving,Applying,Failed};
struct State{Phase phase;u32 index;Browser::Url url;Tcp::Socket* socket;Net::Ipv4 address,nextHopIp;Net::Mac nextHop;char request[768];u32 requestBytes;Browser::HttpResponse response;char body[MaxBody];u32 bodyBytes;u64 started,completed,failed,cssApplied,imagesCached;bool dnsQueryNeeded,arpNeeded;};
extern State state;
void Init(); void Pump();
}
