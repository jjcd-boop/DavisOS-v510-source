#include <cstdio>
#include "Kernel/NetworkCore.hpp"
using namespace Davis;
static bool eq(Net::Ipv4 a,Net::Ipv4 b){for(int i=0;i<4;i++)if(a.b[i]!=b.b[i])return false;return true;}
int main(){Net::Init();Net::Ipv4 local{{192,168,50,10}}, mask{{255,255,255,0}}, gw{{192,168,50,1}}, peer{{192,168,50,99}}, remote{{8,8,8,8}}, zero{};Net::state.config.address=local;Net::state.config.netmask=mask;Net::state.config.gateway=gw;
 if(Net::RouteTo(peer).valid)return 1;Net::state.config.configured=true;auto a=Net::RouteTo(peer);if(!a.valid||!a.onLink||!eq(a.nextHop,peer))return 2;auto b=Net::RouteTo(remote);if(!b.valid||b.onLink||!eq(b.nextHop,gw))return 3;if(Net::RouteTo(zero).valid)return 4;Net::state.config.gateway=zero;if(Net::RouteTo(remote).valid)return 5;
 for(unsigned i=1;i<10000;i++){Net::Ipv4 d{{10,(u8)(i>>8),(u8)i,1}};auto r=Net::RouteTo(d);if(r.valid)return 6;}
 std::puts("PASS network_route configured subnet gateway fail_closed stress=9999");return 0;}
