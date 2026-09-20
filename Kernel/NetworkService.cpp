#include "NetworkService.hpp"
#include "NetworkCore.hpp"
#include "Pci.hpp"
#include "NetworkAdapter.hpp"
#include "Dhcp.hpp"
#include "Arp.hpp"
#include "Dns.hpp"
#include "Icmp.hpp"
#include "Tcp.hpp"
#include "BrowserSession.hpp"
#include "BrowserResourceTransport.hpp"
#include "KernelMemory.hpp"
namespace Davis::NetworkService { State state{}; static u32 cadence=0,arpCadence=0,dnsCadence=0;
void Init(){state={};Net::Init();Arp::Init();Dns::Init();Icmp::Init();Tcp::Init();BrowserSession::Init();BrowserResourceTransport::Init();Pci::State p{};Pci::Scan(p);for(u32 i=0;i<p.count;i++)if(p.devices[i].classCode==0x02){if(p.devices[i].subclass==0x00){state.ethernetControllers++;if(!state.ethernetVendor){state.ethernetVendor=p.devices[i].vendor;state.ethernetDevice=p.devices[i].device;}}else if(p.devices[i].subclass==0x80){state.wifiControllers++;if(!state.wifiVendor){state.wifiVendor=p.devices[i].vendor;state.wifiDevice=p.devices[i].device;}}}if(KernelMemory::ready&&NetworkAdapter::Init(p,KernelMemory::allocator)){state.nicReady=true;Net::state.config.mac=NetworkAdapter::Mac();Net::state.config.link=NetworkAdapter::state.link;Dhcp::Init();}state.ready=true;}
void PumpOnce(){if(!state.ready)return;state.pumps++;if(!state.nicReady)return;Net::Packet in{};for(u32 i=0;i<16&&NetworkAdapter::Receive(in);i++){Net::state.rxPackets++;Arp::OnFrame(in.data,in.bytes);Dhcp::OnFrame(in.data,in.bytes);Dns::OnFrame(in.data,in.bytes);Icmp::OnFrame(in.data,in.bytes);Tcp::OnFrame(in.data,in.bytes);}Arp::Tick();Dhcp::Tick();Dns::Tick();Tcp::Tick();BrowserSession::Pump();if(BrowserSession::state.phase==BrowserSession::Phase::Complete)BrowserResourceTransport::Pump();
 if(Dhcp::state.phase==Dhcp::Phase::Discovering&&((cadence++%100)==0)){Net::Packet p{};if(Dhcp::BuildDiscover(p)&&NetworkAdapter::Send(p.data,p.bytes))Net::state.txPackets++;}
 else if(Dhcp::state.phase==Dhcp::Phase::Requesting){Net::Packet p{};if(Dhcp::BuildRequest(p)&&NetworkAdapter::Send(p.data,p.bytes)){Net::state.txPackets++;Dhcp::state.phase=Dhcp::Phase::AwaitingAck;}}
 state.dhcpBound=Dhcp::state.phase==Dhcp::Phase::Bound;
 if(state.dhcpBound){Net::Mac gw{};if(!Arp::Lookup(Net::state.config.gateway,gw)){if((arpCadence++%100)==0){Net::Packet p{};if(Arp::Request(Net::state.config.gateway,p)&&NetworkAdapter::Send(p.data,p.bytes))Net::state.txPackets++;}}else{state.gatewayResolved=true;if(BrowserSession::state.dnsQueryNeeded&&!Dns::state.waiting&&((dnsCadence++%100)==0)){Net::Packet p{};if(Dns::BuildQuery(BrowserSession::state.url.host,gw,p)&&NetworkAdapter::Send(p.data,p.bytes)){Net::state.txPackets++;BrowserSession::state.dnsQueryNeeded=false;}}}}
 state.dnsResolved=Dns::state.resolved;if(BrowserSession::state.arpNeeded&&Dns::state.resolved){Net::Ipv4 h=BrowserSession::state.nextHopIp;Net::Packet p{};if((arpCadence++%100)==0&&Arp::Request(h,p)&&NetworkAdapter::Send(p.data,p.bytes))Net::state.txPackets++;}state.pingReplies=Icmp::state.replies;
}
}
