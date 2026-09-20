#pragma once
#include "BrowserCore.hpp"
#include "Tcp.hpp"
namespace Davis::BrowserSession {
enum class Phase:u8{Idle,Resolving,Connecting,Sending,Receiving,Complete,Failed,HttpsUnsupported};
static constexpr u32 MaxHistory=8,MaxUrl=384,MaxDecodedBody=8192,MaxRedirects=5;
struct State{Phase phase;Browser::Url url;Tcp::Socket* socket;Net::Ipv4 address,nextHopIp;Net::Mac nextHop;char request[768];u32 requestBytes;Browser::HttpResponse response;char current[MaxUrl];char history[MaxHistory][MaxUrl];u32 historyCount,historyIndex;u64 starts,completes,failures,redirects;u32 redirectDepth,decodedBytes;char decodedBody[MaxDecodedBody];bool ready,dnsQueryNeeded,arpNeeded;};
extern State state;
void Init(); bool Start(const char* url); bool Navigate(const char* url); bool Back(); bool Forward(); bool ResolveLink(const char* href,char*out,u32 cap); void Pump();
}
