#pragma once
#include "../Base/Types.hpp"
#include "NetworkCore.hpp"
namespace Davis::Tcp {
static constexpr u32 MaxSockets=8, MaxPayload=1200, MaxReceive=8192;
enum class Phase:u8 { Closed, SynSent, Established, FinWait, CloseWait, LastAck, Failed };
struct Socket {
 Phase phase; Net::Ipv4 remoteIp; Net::Mac remoteMac; u16 localPort,remotePort;
 u32 sendNext,sendUnacked,recvNext; u16 remoteWindow; u8 receive[MaxReceive]; u32 receiveBytes;
 u32 retransmitTicks,retries; u8 lastFlags; u32 lastBytes; u8 lastPayload[MaxPayload]; bool used,peerClosed,pendingAck,pendingRetransmit;
};
struct State { Socket sockets[MaxSockets]; u64 opens,segmentsTx,segmentsRx,retransmits,resets,checksumErrors; bool ready; };
extern State state;
void Init(); void Release(Socket&); Socket* Open(Net::Ipv4 remote,Net::Mac mac,u16 port); bool BuildPending(Socket&,Net::Packet&); bool SendData(Socket&,const void*,u32,Net::Packet&); bool Close(Socket&,Net::Packet&); void OnFrame(const u8*,u32); void Tick();
}
