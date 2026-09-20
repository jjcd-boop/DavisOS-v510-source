#include "Arp.hpp"
namespace Davis::Arp { State state{}; static u16 r16(const u8*p){return (u16)((u16)p[0]<<8|p[1]);} static bool eq(Net::Ipv4 a,Net::Ipv4 b){for(u32 i=0;i<4;i++)if(a.b[i]!=b.b[i])return false;return true;}
void Init(){state={};} void Tick(){for(auto&e:state.entries)if(e.valid&&++e.age>60000)e.valid=false;}
static void learn(Net::Ipv4 ip,Net::Mac mac){for(auto&e:state.entries)if(e.valid&&eq(e.ip,ip)){e.mac=mac;e.age=0;return;}for(auto&e:state.entries)if(!e.valid){e={ip,mac,0,true};state.learned++;return;}state.entries[0]={ip,mac,0,true};state.learned++;}
void OnFrame(const u8*f,u32 n){if(!f||n<42||r16(f+12)!=0x0806)return;const u8*a=f+14;if(r16(a)!=1||r16(a+2)!=0x0800||a[4]!=6||a[5]!=4)return;Net::Mac m{};Net::Ipv4 ip{};for(u32 i=0;i<6;i++)m.b[i]=a[8+i];for(u32 i=0;i<4;i++)ip.b[i]=a[14+i];learn(ip,m);if(r16(a+6)==2)state.replies++;}
bool Lookup(Net::Ipv4 ip,Net::Mac&m){for(auto&e:state.entries)if(e.valid&&eq(e.ip,ip)){m=e.mac;return true;}return false;} bool Request(Net::Ipv4 ip,Net::Packet&o){state.requests++;return Net::BuildArpRequest(o,Net::state.config.mac,Net::state.config.address,ip);}
}
