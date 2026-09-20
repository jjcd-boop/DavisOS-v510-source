#include "XhciPorts.hpp"
namespace Davis::Xhci {
static constexpr u32 PORTSC_BASE=0x400,PORT_STRIDE=0x10;
static constexpr u32 CCS=1u<<0,PED=1u<<1,PR=1u<<4,PP=1u<<9;
static inline volatile u32* PortSc(const State&s,u32 i){return (volatile u32*)(uptr)(s.mmio+s.capLength+PORTSC_BASE+i*PORT_STRIDE);}
static PortInfo Decode(u32 i,u32 v){PortInfo p{};p.port=(u8)(i+1);p.portsc=v;p.connected=(v&CCS)!=0;p.enabled=(v&PED)!=0;p.powered=(v&PP)!=0;p.speed=(u8)((v>>10)&0xF);return p;}
void ScanPorts(const State&s,const ControllerState&c,PortState&o){o={};o.result=PortResult::InvalidState;if(!s.capabilitiesValid||!c.running)return;u32 n=s.maxPorts;if(n>32)n=32;for(u32 i=0;i<n;i++)o.ports[o.count++]=Decode(i,*PortSc(s,i));o.result=PortResult::Ok;}
PortResult ResetPort(const State&s,const ControllerState&c,u8 port,PortState&o,u32 spins){ScanPorts(s,c,o);if(o.result!=PortResult::Ok)return o.result;if(!port||port>o.count||!o.ports[port-1].connected)return o.result=PortResult::NoConnectedPort;u32 idx=(u32)port-1;
 volatile u32*r=PortSc(s,idx);u32 v=*r; // preserve control bits; write-1 change bits are explicitly cleared below
 v&=~((1u<<17)|(1u<<18)|(1u<<20)|(1u<<21)|(1u<<22)|(1u<<23));*r=v|PR;
 while(spins--){u32 q=*r;if(!(q&PR)){o.ports[idx]=Decode(idx,q);o.selectedPort=(u8)(idx+1);return o.result=(q&PED)?PortResult::Ok:PortResult::EnableTimeout;}__asm__ __volatile__("pause" ::: "memory");}
 return o.result=PortResult::ResetTimeout;}
PortResult ResetFirstConnectedPort(const State&s,const ControllerState&c,PortState&o,u32 spins){ScanPorts(s,c,o);if(o.result!=PortResult::Ok)return o.result;for(u32 i=0;i<o.count;i++)if(o.ports[i].connected)return ResetPort(s,c,o.ports[i].port,o,spins);return o.result=PortResult::NoConnectedPort;}
}
