#include "AmlLite.hpp"
namespace Davis::AmlLite {
static bool eq4(const u8*p,const char*s){return p[0]==(u8)s[0]&&p[1]==(u8)s[1]&&p[2]==(u8)s[2]&&p[3]==(u8)s[3];}
static bool prefix(const char*s,const char*p){for(;*p;p++,s++)if(*s!=*p)return false;return true;}
static bool likely(const char*s){return prefix(s,"ELAN")||prefix(s,"SYNA")||prefix(s,"SYN")||prefix(s,"PNP0C50")||prefix(s,"MSFT")||prefix(s,"DLL")||prefix(s,"ACPI");}
void Scan(const Acpi::State&a,State&s){s={};if(!a.dsdtPresent||!a.dsdt)return;const u8*b=(const u8*)a.dsdt;u32 len=*(const u32*)(b+4);if(len<36||len>(4u<<20))return;s.dsdtScanned=true;const u8*p=b+36,*e=b+len;for(;p+10<e;p++){
 if(*p==0x5B&&p[1]==0x82)s.deviceCount++; // ExtOp DeviceOp
 if(eq4(p,"_HID")){
   const u8*q=p+4;for(u32 skip=0;skip<8&&q<e;skip++,q++){if(*q==0x0D){q++;HidDevice d{};u32 n=0;while(q<e&&*q&&n<15)d.hid[n++]=(char)*q++;d.hid[n]=0;for(const u8*r=p;r<e&&r<p+192;r++)if(r+4<e&&eq4(r,"_CRS"))d.hasCrs=true;d.likelyTouchpad=likely(d.hid);if(s.hidCount<16)s.devices[s.hidCount++]=d;if(d.likelyTouchpad)s.touchpadCandidates++;break;}}
 }
}}
}
