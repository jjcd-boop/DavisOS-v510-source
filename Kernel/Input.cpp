#include "Input.hpp"
#include "Ps2Diagnostics.hpp"
#include "../Arch/x86_64/Io.hpp"
#include "UsbRuntime.hpp"
#include "UsbHid.hpp"
#include "I2cHid.hpp"
namespace Davis::Input {
static char chars[64]; static u8 head=0,tail=0; static bool numLock=true,capsLock=false; static UsbHid::Keyboard usbK{}; static UsbHid::Mouse usbM{};
static void push(char c){u8 n=(u8)((head+1)&63);if(n!=tail){chars[head]=c;head=n;}}
char PopChar(){if(tail==head)return 0;char c=chars[tail];tail=(u8)((tail+1)&63);return c;}
static char key(u8 s,bool sh){
 // IBM PC/AT Set-1 make-code map. Keep this explicit: the old compact string was
 // offset by two positions, which turned TEST into YRGY and ZX into BN.
 char c=0;
 switch(s){
  case 0x01:return 27; case 0x0E:return '\b'; case 0x0F:return '\t'; case 0x1C:return '\n'; case 0x39:return ' ';
  case 0x02:c='1';break;case 0x03:c='2';break;case 0x04:c='3';break;case 0x05:c='4';break;case 0x06:c='5';break;
  case 0x07:c='6';break;case 0x08:c='7';break;case 0x09:c='8';break;case 0x0A:c='9';break;case 0x0B:c='0';break;
  case 0x0C:c='-';break;case 0x0D:c='=';break;case 0x10:c='q';break;case 0x11:c='w';break;case 0x12:c='e';break;
  case 0x13:c='r';break;case 0x14:c='t';break;case 0x15:c='y';break;case 0x16:c='u';break;case 0x17:c='i';break;
  case 0x18:c='o';break;case 0x19:c='p';break;case 0x1A:c='[';break;case 0x1B:c=']';break;case 0x1E:c='a';break;
  case 0x1F:c='s';break;case 0x20:c='d';break;case 0x21:c='f';break;case 0x22:c='g';break;case 0x23:c='h';break;
  case 0x24:c='j';break;case 0x25:c='k';break;case 0x26:c='l';break;case 0x27:c=';';break;case 0x28:c='\'';break;
  case 0x29:c='`';break;case 0x2B:c='\\';break;case 0x2C:c='z';break;case 0x2D:c='x';break;case 0x2E:c='c';break;
  case 0x2F:c='v';break;case 0x30:c='b';break;case 0x31:c='n';break;case 0x32:c='m';break;case 0x33:c=',';break;
  case 0x34:c='.';break;case 0x35:c='/';break;
  case 0x4A:return '-';case 0x4E:return '+';case 0x53:return '.';case 0x52:return '0';case 0x4F:return '1';case 0x50:return '2';case 0x51:return '3';case 0x4B:return '4';case 0x4C:return '5';case 0x4D:return '6';case 0x47:return '7';case 0x48:return '8';case 0x49:return '9';
 }
 if(!c)return 0;if(!sh)return c;
 if(c>='a'&&c<='z')return (char)(c-'a'+'A');
 switch(c){case '1':return '!';case '2':return '@';case '3':return '#';case '4':return '$';case '5':return '%';case '6':return '^';case '7':return '&';case '8':return '*';case '9':return '(';case '0':return ')';case '-':return '_';case '=':return '+';case '[':return '{';case ']':return '}';case '\\':return '|';case ';':return ':';case '\'':return '"';case '`':return '~';case ',':return '<';case '.':return '>';case '/':return '?';default:return c;}
}
void Init(State&s,u32 w,u32 h){s={};Ps2Diagnostics::Init();UsbHid::Init(usbK);usbM={};s.mouseX=w/2;s.mouseY=h/2;
}
void Poll(State&s,u32 w,u32 h){
 s.prevLeft=s.left; s.pressLatched=false; s.releaseLatched=false;
 char uk=0;bool um=false;UsbRuntime::PollHid(usbK,usbM,uk,um);if(uk){push(uk);s.keyEvents++;}
 if(um){s.mouseX+=usbM.dx;s.mouseY-=usbM.dy;bool nl=usbM.left;if(nl&&!s.left)s.pressLatched=true;if(!nl&&s.left)s.releaseLatched=true;s.left=nl;}
 i32 idx=0,idy=0;bool il=false;if(I2cHid::Poll(idx,idy,il)){s.mouseX+=idx;s.mouseY-=idy;if(il&&!s.left)s.pressLatched=true;if(!il&&s.left)s.releaseLatched=true;s.left=il;}
 for(int count=0;count<64;count++){
  Ps2Diagnostics::state.polls++;u8 st=Io::In8(0x64);Ps2Diagnostics::state.liveStatus=st;if(!(st&1))break;u8 v=Io::In8(0x60);Ps2Diagnostics::NoteByte(st,v);
  if(st&0x20){
   // PS/2 packet byte 0 always has bit 3 set. Re-synchronise on that invariant.
   if(s.mouseIndex==0 && !(v&0x08))continue;
   s.mousePacket[s.mouseIndex++]=v;
   if(s.mouseIndex==3){s.mouseIndex=0;u8 b=s.mousePacket[0];if(!(b&8))continue;
    // Discard overflow packets rather than turning them into huge cursor jumps.
    if(b&0xC0)continue;
    i64 dx=(i8)s.mousePacket[1],dy=(i8)s.mousePacket[2];
    s.mouseX+=dx;s.mouseY-=dy;bool nl=(b&1)!=0;if(nl&&!s.left)s.pressLatched=true;if(!nl&&s.left)s.releaseLatched=true;s.left=nl;s.right=(b&2)!=0;s.middle=(b&4)!=0;s.mousePackets++;
   }
  }else{
   if(v==0xE0){s.extended=true;continue;} if(v==0xE1){s.extended=false;continue;}
   bool rel=(v&0x80)!=0;u8 sc=v&0x7F;
   if(sc==0x2A||sc==0x36){s.shift=!rel;s.extended=false;continue;}
   if(sc==0x1D){s.ctrl=!rel;s.extended=false;continue;} if(sc==0x38){s.alt=!rel;s.extended=false;continue;}
   if(!rel){
    if(sc==0x45&&!s.extended){numLock=!numLock;s.extended=false;continue;}if(sc==0x3A){capsLock=!capsLock;s.extended=false;continue;}
    char c=0;
    // Extended keypad Enter and slash are distinct from navigation keys.
    if(s.extended&&sc==0x1C)c='\n'; else if(s.extended&&sc==0x35)c='/'; else c=key(sc,s.shift);
    if(c){if(c>='a'&&c<='z'&&capsLock)c=(char)(c-'a'+'A');if(sc>=0x47&&sc<=0x53&&!numLock&&!s.extended)c=0;if(c){push(c);s.keyEvents++;}}
   }
   s.extended=false;
  }
 }
 if(s.mouseX<0)s.mouseX=0;if(s.mouseY<0)s.mouseY=0;if(s.mouseX>=(i64)w)s.mouseX=w-1;if(s.mouseY>=(i64)h)s.mouseY=h-1;
}
bool Clicked(const State&s){return s.pressLatched || (s.left&&!s.prevLeft);}
bool Released(const State&s){return s.releaseLatched || (!s.left&&s.prevLeft);}
}
