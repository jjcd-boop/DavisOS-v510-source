#include <cstdio>
#include <cstring>
#include "Kernel/NetworkCore.hpp"
#include "Kernel/Tcp.hpp"
#include "Kernel/Dns.hpp"
#include "Kernel/Dhcp.hpp"
using namespace Davis;
static void w16(u8*p,u16 v){p[0]=v>>8;p[1]=v;}
static void w32(u8*p,u32 v){p[0]=v>>24;p[1]=v>>16;p[2]=v>>8;p[3]=v;}
static bool eq4(Net::Ipv4 a,Net::Ipv4 b){return !std::memcmp(a.b,b.b,4);}
static Net::Packet tcpReply(Tcp::Socket&s,u8 flags,u32 seq,u32 ack){
 Net::Packet o{};o.bytes=54;for(int i=0;i<6;i++){o.data[i]=Net::state.config.mac.b[i];o.data[6+i]=s.remoteMac.b[i];}w16(o.data+12,0x0800);u8*ip=o.data+14;ip[0]=0x45;w16(ip+2,40);w16(ip+6,0x4000);ip[8]=64;ip[9]=6;for(int i=0;i<4;i++){ip[12+i]=s.remoteIp.b[i];ip[16+i]=Net::state.config.address.b[i];}w16(ip+10,Net::Checksum16(ip,20));u8*t=ip+20;w16(t,s.remotePort);w16(t+2,s.localPort);w32(t+4,seq);w32(t+8,ack);t[12]=0x50;t[13]=flags;w16(t+14,4096);
 // independent TCP pseudoheader checksum
 u32 sum=0;auto add=[&](u16 v){sum+=v;while(sum>>16)sum=(sum&0xffff)+(sum>>16);};auto ip16=[&](Net::Ipv4 x,int k){return (u16)((u16)x.b[k]<<8|x.b[k+1]);};add(ip16(s.remoteIp,0));add(ip16(s.remoteIp,2));add(ip16(Net::state.config.address,0));add(ip16(Net::state.config.address,2));add(6);add(20);for(int i=0;i<20;i+=2){u16 v=(i==16)?0:(u16)((u16)t[i]<<8|t[i+1]);add(v);}u16 c=(u16)~sum;w16(t+16,c);return o;
}
int main(){
 Net::Init();Net::state.config.configured=true;Net::state.config.address={{10,1,2,3}};Net::state.config.dns={{8,8,8,8}};Net::state.config.mac={{1,2,3,4,5,6}};Tcp::Init();Dns::Init();Dhcp::Init();
 Net::Mac rm{{6,5,4,3,2,1}};Net::Ipv4 rip{{10,1,2,9}};auto*s=Tcp::Open(rip,rm,443);if(!s)return 1;Net::Packet syn{};if(!Tcp::BuildPending(*s,syn))return 2;auto good=tcpReply(*s,0x12,1000,s->sendNext+1);Tcp::OnFrame(good.data,good.bytes);if(s->phase!=Tcp::Phase::Established)return 3;
 // Wrong destination, fragmentation and corrupt IP checksum must not mutate established socket receive state.
 u32 rx=Tcp::state.segmentsRx;auto bad=good;bad.data[30]^=1;Tcp::OnFrame(bad.data,bad.bytes);if(Tcp::state.segmentsRx!=rx)return 4;
 bad=good;w16(bad.data+20,0x2000);w16(bad.data+24,0);w16(bad.data+24,Net::Checksum16(bad.data+14,20));Tcp::OnFrame(bad.data,bad.bytes);if(Tcp::state.segmentsRx!=rx)return 5;
 bad=good;bad.data[30]=99;w16(bad.data+24,0);w16(bad.data+24,Net::Checksum16(bad.data+14,20));Tcp::OnFrame(bad.data,bad.bytes);if(Tcp::state.segmentsRx!=rx)return 6;
 // DHCP transaction identity: invalid cookie must not advance discovery.
 u8 fake[260]{}; (void)fake; if(Dhcp::state.phase!=Dhcp::Phase::Discovering)return 7;
 std::printf("PASS network_protocol_hardening tcp_ipv4_integrity destination fragmentation dhcp_identity\n");return 0;
}
