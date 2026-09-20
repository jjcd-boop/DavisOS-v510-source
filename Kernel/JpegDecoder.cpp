#include "JpegDecoder.hpp"
#include "BrowserImage.hpp"
namespace Davis::JpegDecoder {
struct Huff{u8 bits[17];u8 vals[256];u16 count;};
struct Comp{u8 id,h,v,tq,td,ta;int dc;};
struct Bits{const u8*p,*e;u32 buf,n;bool get(u32 k,u32&v){while(n<k){if(p>=e)return false;u8 b=*p++;if(b==0xff){if(p>=e)return false;if(*p==0x00)p++;else return false;}buf=(buf<<8)|b;n+=8;}v=(buf>>(n-k))&((1u<<k)-1);n-=k;return true;}};
static u16 be16(const u8*p){return (u16)(((u16)p[0]<<8)|p[1]);}
static int extend(u32 v,u8 n){if(!n)return 0;u32 t=1u<<(n-1);return v<t?(int)v-(int)((1u<<n)-1):(int)v;}
static bool sym(Bits&b,const Huff&h,u32&s){u32 code=0,first=0,idx=0;for(u8 len=1;len<=16;len++){u32 x;if(!b.get(1,x))return false;code=(code<<1)|x;u32 cnt=h.bits[len];if(code-first<cnt){s=h.vals[idx+code-first];return true;}idx+=cnt;first=(first+cnt)<<1;}return false;}
static const u8 zig[64]={0,1,8,16,9,2,3,10,17,24,32,25,18,11,4,5,12,19,26,33,40,48,41,34,27,20,13,6,7,14,21,28,35,42,49,56,57,50,43,36,29,22,15,23,30,37,44,51,58,59,52,45,38,31,39,46,53,60,61,54,47,55,62,63};
static int clamp(int x){return x<0?0:x>255?255:x;}
static void idct(const int*in,u8*out){ // integer-friendly direct 8x8 IDCT; slow but bounded and dependable for bring-up
 static const int C[8][8]={{181,251,237,213,181,142,98,50},{181,213,98,-50,-181,-251,-237,-142},{181,142,-98,-251,-181,50,237,213},{181,50,-237,-142,181,213,-98,-251},{181,-50,-237,142,181,-213,-98,251},{181,-142,-98,251,-181,-50,237,-213},{181,-213,98,50,-181,251,-237,142},{181,-251,237,-213,181,-142,98,-50}};
 for(int y=0;y<8;y++)for(int x=0;x<8;x++){long long s=0;for(int v=0;v<8;v++)for(int u=0;u<8;u++)s+=(long long)in[v*8+u]*C[x][u]*C[y][v];out[y*8+x]=(u8)clamp((int)(s/(256*256*2))+128);}}
static bool block(Bits&b,Huff&hd,Huff&ha,const u16*q,int&dc,u8*out){int coef[64]={};u32 s,v=0;if(!sym(b,hd,s)||s>11)return false;if(s&&!b.get(s,v))return false;dc+=extend(v,(u8)s);coef[0]=dc*(int)q[0];int k=1;while(k<64){if(!sym(b,ha,s))return false;if(s==0)break;if(s==0xf0){k+=16;continue;}u32 run=s>>4,n=s&15;k+=run;if(k>=64||!n||n>10||!b.get(n,v))return false;coef[zig[k]]=extend(v,(u8)n)*(int)q[zig[k]];k++;}idct(coef,out);return true;}
static u8 sample(const u8*blocks,u8 h,u8 v,u8 maxh,u8 maxv,int x,int y){int sx=x*h/maxh,sy=y*v/maxv;int bx=sx>>3,by=sy>>3;return blocks[(by*h+bx)*64+(sy&7)*8+(sx&7)];}
bool Decode(const u8*d,u32 n,BrowserImage::Image&o){if(!d||n<4||d[0]!=0xff||d[1]!=0xd8)return false;u16 qt[4][64]={{0}};bool qok[4]={};Huff dc[4]{},ac[4]{};bool dok[4]={},aok[4]={};Comp c[3]{};u8 nc=0,maxh=1,maxv=1;u16 w=0,h=0;const u8*scan=nullptr,*end=d+n;u32 pos=2;
 while(pos+4<=n){if(d[pos++]!=0xff)continue;while(pos<n&&d[pos]==0xff)pos++;if(pos>=n)return false;u8 m=d[pos++];if(m==0xd9)break;if(m==0xda){if(pos+2>n)return false;u16 L=be16(d+pos);if(L<6||pos+L>n)return false;u8 ns=d[pos+2];if(ns!=nc)return false;u32 q=pos+3;for(u8 i=0;i<ns;i++){u8 id=d[q++],sel=d[q++];bool found=false;for(u8 j=0;j<nc;j++)if(c[j].id==id){c[j].td=sel>>4;c[j].ta=sel&15;found=true;}if(!found)return false;}scan=d+pos+L;break;}if(m==0xd8||m==0x01||(m>=0xd0&&m<=0xd7))continue;if(pos+2>n)return false;u16 L=be16(d+pos);if(L<2||pos+L>n)return false;const u8*p=d+pos+2;u32 z=L-2;
  if(m==0xdb){while(z){if(z<65)return false;u8 x=*p++;z--;if(x>>4)return false;u8 t=x&15;if(t>3)return false;for(int k=0;k<64;k++)qt[t][zig[k]]=*p++;z-=64;qok[t]=true;}}
  else if(m==0xc4){while(z){if(z<17)return false;u8 x=*p++;z--;u8 tc=x>>4,th=x&15;if(tc>1||th>3)return false;Huff&hh=tc?ac[th]:dc[th];hh.count=0;for(int k=1;k<=16;k++){hh.bits[k]=*p++;hh.count+=hh.bits[k];}z-=16;if(hh.count>256||z<hh.count)return false;for(u16 k=0;k<hh.count;k++)hh.vals[k]=*p++;z-=hh.count;(tc?aok[th]:dok[th])=true;}}
  else if(m==0xc0){if(z<6)return false;u8 precision=p[0];h=be16(p+1);w=be16(p+3);nc=p[5];if(precision!=8||!(nc==1||nc==3)||z<6+3u*nc)return false;for(u8 i=0;i<nc;i++){c[i].id=p[6+i*3];c[i].h=p[7+i*3]>>4;c[i].v=p[7+i*3]&15;c[i].tq=p[8+i*3];if(!c[i].h||!c[i].v||c[i].h>2||c[i].v>2||c[i].tq>3)return false;if(c[i].h>maxh)maxh=c[i].h;if(c[i].v>maxv)maxv=c[i].v;}}
  else if(m==0xc2)return false; // progressive JPEG intentionally deferred
  pos+=L;
 }
 if(!scan||!w||!h||!nc||!BrowserImage::AllocatePixels(o,w,h))return false;for(u8 i=0;i<nc;i++)if(!qok[c[i].tq]||!dok[c[i].td]||!aok[c[i].ta])return false;
 Bits b{scan,end,0,0};u32 mw=8*maxh,mh=8*maxv;u8 blocks[3][4*64];for(u32 my=0;my<h;my+=mh)for(u32 mx=0;mx<w;mx+=mw){for(u8 ci=0;ci<nc;ci++)for(u8 by=0;by<c[ci].v;by++)for(u8 bx=0;bx<c[ci].h;bx++)if(!block(b,dc[c[ci].td],ac[c[ci].ta],qt[c[ci].tq],c[ci].dc,&blocks[ci][(by*c[ci].h+bx)*64]))return false;for(u32 yy=0;yy<mh&&my+yy<h;yy++)for(u32 xx=0;xx<mw&&mx+xx<w;xx++){int Y=sample(blocks[0],c[0].h,c[0].v,maxh,maxv,xx,yy),Cb=128,Cr=128;if(nc==3){Cb=sample(blocks[1],c[1].h,c[1].v,maxh,maxv,xx,yy);Cr=sample(blocks[2],c[2].h,c[2].v,maxh,maxv,xx,yy);}int cb=Cb-128,cr=Cr-128;int R=Y+(359*cr>>8),G=Y-((88*cb+183*cr)>>8),B=Y+(454*cb>>8);o.pixels[(my+yy)*w+mx+xx]=0xff000000u|((u32)clamp(R)<<16)|((u32)clamp(G)<<8)|(u32)clamp(B);}}
 o.width=w;o.height=h;o.pixelCount=(u32)w*h;o.decoded=true;return true;}
}
