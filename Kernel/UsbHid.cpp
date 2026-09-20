#include "UsbHid.hpp"
namespace Davis::UsbHid {
void Init(Keyboard&k){k={};}
static bool Was(const Keyboard&k,u8 c){for(int i=0;i<6;i++)if(k.previous[i]==c)return true;return false;}
static char Key(u8 c,bool sh){
 if(c>=4&&c<=29)return (char)((sh?'A':'a')+(c-4));
 if(c>=30&&c<=38)return sh?")!@#$%^&*("[c-30]:(char)('1'+c-30);
 if(c==39)return sh?')':'0'; if(c==44)return ' '; if(c==40)return '\n'; if(c==42)return '\b';
 const char lo[]="-=[]\\;'`,./"; const char hi[]="_+{}|:\"~<>?";
 if(c>=45&&c<=56)return sh?hi[c-45]:lo[c-45]; return 0;
}
char DecodeBootKeyboard(Keyboard&k,const u8 r[8]){
 k.ctrl=(r[0]&0x11)!=0;k.shift=(r[0]&0x22)!=0;k.alt=(r[0]&0x44)!=0;char out=0;
 for(int i=2;i<8&&!out;i++)if(r[i]&&!Was(k,r[i]))out=Key(r[i],k.shift);
 for(int i=0;i<6;i++)k.previous[i]=r[i+2];return out;
}
void DecodeBootMouse(Mouse&m,const u8*r,usize n){m={};if(n<3)return;m.left=(r[0]&1)!=0;m.right=(r[0]&2)!=0;m.middle=(r[0]&4)!=0;m.dx=(int)(signed char)r[1];m.dy=(int)(signed char)r[2];if(n>3)m.wheel=(int)(signed char)r[3];}
}
