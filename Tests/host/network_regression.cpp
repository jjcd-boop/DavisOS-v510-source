#include "Kernel/NetworkCore.hpp"
#include <cstdio>
using namespace Davis;
int main(){Net::Init(); Net::Packet p{}; Net::Mac a{{1,2,3,4,5,6}},b{{6,5,4,3,2,1}}; Net::Ipv4 s{{10,0,0,1}},d{{10,0,0,2}}; const char msg[]="davis-network-stress";
 for(int i=0;i<10000;i++){if(!Net::BuildIpv4Udp(p,a,b,s,d,1234,4321,msg,sizeof(msg)))return 1;Net::Ipv4 rs{},rd{};u16 sp=0,dp=0;const u8*q=nullptr;u32 n=0;if(!Net::ParseIpv4Udp(p.data,p.bytes,rs,rd,sp,dp,q,n)||sp!=1234||dp!=4321||n!=sizeof(msg))return 2;}
 auto bad=p;bad.data[14+8]^=1; Net::Ipv4 rs{},rd{};u16 sp,dp;const u8*q;u32 n;if(Net::ParseIpv4Udp(bad.data,bad.bytes,rs,rd,sp,dp,q,n))return 3;
 bad=p;bad.data[14+6]=0x20;if(Net::ParseIpv4Udp(bad.data,bad.bytes,rs,rd,sp,dp,q,n))return 4;
 bad=p;bad.data[42]^=0x40;if(Net::ParseIpv4Udp(bad.data,bad.bytes,rs,rd,sp,dp,q,n))return 5;
 if(Net::BuildIpv4Udp(p,a,b,s,d,1,2,msg,Net::MaxFrame))return 6;
 std::printf("PASS network_regression udp=10000 ip_checksum udp_checksum fragmentation bounds\n");return 0;}
