#include "Icmp.hpp"
namespace Davis::Icmp {State state{};static void w16(u8*p,u16 v){p[0]=v>>8;p[1]=v;}static u16 r16(const u8*p){return (u16)((u16)p[0]<<8|p[1]);}void Init(){state={};}
bool BuildEcho(Net::Ipv4 dip,Net::Mac dm,Net::Packet&o){o={};o.bytes=14+20+16;for(u32 i=0;i<6;i++){o.data[i]=dm.b[i];o.data[6+i]=Net::state.config.mac.b[i];}w16(o.data+12,0x0800);u8*ip=o.data+14;ip[0]=0x45;w16(ip+2,36);w16(ip+4,Net::state.nextIpId++);w16(ip+6,0x4000);ip[8]=64;ip[9]=1;for(u32 i=0;i<4;i++){ip[12+i]=Net::state.config.address.b[i];ip[16+i]=dip.b[i];}w16(ip+10,Net::Checksum16(ip,20));u8*ic=ip+20;ic[0]=8;ic[1]=0;w16(ic+4,0xDA15);w16(ic+6,++state.sequence);for(u32 i=8;i<16;i++)ic[i]=(u8)i;w16(ic+2,Net::Checksum16(ic,16));state.sent++;return true;}
void OnFrame(const u8*f,u32 n){if(!f||n<42||r16(f+12)!=0x0800)return;const u8*ip=f+14;u32 h=(ip[0]&15)*4;if((ip[0]>>4)!=4||ip[9]!=1||14+h+8>n)return;const u8*ic=ip+h;if(ic[0]==0&&r16(ic+4)==0xDA15)state.replies++;}
}
