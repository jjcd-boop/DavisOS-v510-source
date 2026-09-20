#include "NetworkCore.hpp"
namespace Davis::Net {
State state{};
static void be16(u8*p,u16 v){p[0]=(u8)(v>>8);p[1]=(u8)v;} static u16 rd16(const u8*p){return (u16)((u16)p[0]<<8|p[1]);}
void Init(){state={};state.nextIpId=1;state.nextEphemeral=49152;state.ready=true;}
u16 Checksum16(const void* data,u32 bytes){const u8*p=(const u8*)data;u32 sum=0;while(bytes>1){sum+=((u16)p[0]<<8)|p[1];p+=2;bytes-=2;}if(bytes)sum+=(u16)p[0]<<8;while(sum>>16)sum=(sum&0xffff)+(sum>>16);return (u16)~sum;}
bool IsZero(Ipv4 a){return !(a.b[0]|a.b[1]|a.b[2]|a.b[3]);}
bool SameSubnet(Ipv4 a,Ipv4 b,Ipv4 mask){for(u32 i=0;i<4;i++)if((a.b[i]&mask.b[i])!=(b.b[i]&mask.b[i]))return false;return true;}
RouteDecision RouteTo(Ipv4 d){RouteDecision r{};if(!state.config.configured||IsZero(d))return r;if(SameSubnet(state.config.address,d,state.config.netmask)){r.nextHop=d;r.onLink=true;r.valid=true;return r;}if(!IsZero(state.config.gateway)){r.nextHop=state.config.gateway;r.valid=true;}return r;}
bool BuildArpRequest(Packet&o,Mac sm,Ipv4 sip,Ipv4 tip){o={};o.bytes=42;for(u32 i=0;i<6;i++){o.data[i]=0xff;o.data[6+i]=sm.b[i];}be16(o.data+12,0x0806);u8*a=o.data+14;be16(a,1);be16(a+2,0x0800);a[4]=6;a[5]=4;be16(a+6,1);for(u32 i=0;i<6;i++)a[8+i]=sm.b[i];for(u32 i=0;i<4;i++)a[14+i]=sip.b[i];for(u32 i=0;i<6;i++)a[18+i]=0;for(u32 i=0;i<4;i++)a[24+i]=tip.b[i];return true;}
static u16 udpChecksum(Ipv4 sip,Ipv4 dip,const u8*u,u16 len){u32 sum=0;auto add16=[&](u16 v){sum+=v;while(sum>>16)sum=(sum&0xffff)+(sum>>16);};add16((u16)((sip.b[0]<<8)|sip.b[1]));add16((u16)((sip.b[2]<<8)|sip.b[3]));add16((u16)((dip.b[0]<<8)|dip.b[1]));add16((u16)((dip.b[2]<<8)|dip.b[3]));add16(17);add16(len);for(u32 i=0;i<len;i+=2){u16 v=(u16)u[i]<<8;if(i+1<len)v|=u[i+1];if(i==6)v=0;add16(v);}u16 c=(u16)~sum;return c?c:0xffff;}
bool BuildIpv4Udp(Packet&o,Mac sm,Mac dm,Ipv4 sip,Ipv4 dip,u16 sp,u16 dp,const void* payload,u32 n){if(n>MaxFrame-42)return false;o={};o.bytes=42+n;for(u32 i=0;i<6;i++){o.data[i]=dm.b[i];o.data[6+i]=sm.b[i];}be16(o.data+12,0x0800);u8*ip=o.data+14;ip[0]=0x45;ip[1]=0;be16(ip+2,(u16)(28+n));be16(ip+4,state.nextIpId++);be16(ip+6,0x4000);ip[8]=64;ip[9]=17;for(u32 i=0;i<4;i++){ip[12+i]=sip.b[i];ip[16+i]=dip.b[i];}be16(ip+10,Checksum16(ip,20));u8*u=ip+20;be16(u,sp);be16(u+2,dp);be16(u+4,(u16)(8+n));be16(u+6,0);const u8*s=(const u8*)payload;for(u32 i=0;i<n;i++)u[8+i]=s[i];be16(u+6,udpChecksum(sip,dip,u,(u16)(8+n)));return true;}
bool ParseIpv4Udp(const u8*f,u32 n,Ipv4&s,Ipv4&d,u16&sp,u16&dp,const u8*&p,u32&pn){
 if(!f||n<42||rd16(f+12)!=0x0800)return false;const u8*ip=f+14;u32 ihl=(ip[0]&15)*4;
 if((ip[0]>>4)!=4||ihl<20||ihl>60||14+ihl+8>n||ip[9]!=17)return false;
 u16 total=rd16(ip+2);if(total<ihl+8||14+total>n)return false;
 // Reject fragmented datagrams until IP reassembly exists, and reject corrupted IPv4 headers.
 u16 frag=rd16(ip+6);if((frag&0x3fffu)!=0)return false;if(Checksum16(ip,ihl)!=0)return false;
 for(u32 i=0;i<4;i++){s.b[i]=ip[12+i];d.b[i]=ip[16+i];}const u8*u=ip+ihl;sp=rd16(u);dp=rd16(u+2);u16 ul=rd16(u+4);
 if(ul<8||ihl+ul>total)return false;u16 wire=rd16(u+6);if(wire&&udpChecksum(s,d,u,ul)!=wire)return false;p=u+8;pn=ul-8;return true;
}
}
