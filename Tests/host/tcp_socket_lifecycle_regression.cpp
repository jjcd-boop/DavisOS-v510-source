#include <cstdio>
#include "Kernel/NetworkCore.hpp"
#include "Kernel/Tcp.hpp"
using namespace Davis;
int main(){Net::Init();Net::state.config.configured=true;Tcp::Init();Net::Ipv4 ip{{1,2,3,4}};Net::Mac m{{1,2,3,4,5,6}};for(unsigned round=0;round<5000;round++){Tcp::Socket*s[Tcp::MaxSockets]{};for(unsigned i=0;i<Tcp::MaxSockets;i++){s[i]=Tcp::Open(ip,m,80);if(!s[i])return 1;}if(Tcp::Open(ip,m,80))return 2;for(unsigned i=0;i<Tcp::MaxSockets;i++)Tcp::Release(*s[i]);}if(Tcp::state.opens!=5000ull*Tcp::MaxSockets)return 3;std::printf("PASS tcp_socket_lifecycle cycles=5000 opens=%llu capacity=%u\n",(unsigned long long)Tcp::state.opens,Tcp::MaxSockets);return 0;}
