#include "../Kernel/UsbHid.hpp"
#include "../Kernel/ShellLayout.hpp"
int main(){
 Davis::UsbHid::Keyboard k;Davis::UsbHid::Init(k);
 Davis::u8 a[8]={0,0,4,0,0,0,0,0}; if(Davis::UsbHid::DecodeBootKeyboard(k,a)!='a')return 1;
 Davis::u8 held[8]={0,0,4,0,0,0,0,0}; if(Davis::UsbHid::DecodeBootKeyboard(k,held)!=0)return 2;
 Davis::u8 A[8]={2,0,4,0,0,0,0,0}; k.previous[0]=0;if(Davis::UsbHid::DecodeBootKeyboard(k,A)!='A')return 3;
 Davis::UsbHid::Mouse m;Davis::u8 mr[3]={1,5,253};Davis::UsbHid::DecodeBootMouse(m,mr,3);
 if(!m.left||m.dx!=5||m.dy!=-3)return 4;
 Davis::Graphics::Surface s{};s.width=1920;s.height=1080;auto l=Davis::ShellLayout::Compute(s);
 if(l.taskbar.y<=900||!Davis::ShellLayout::Hit(l.start,l.start.x+2,l.start.y+2))return 5;
 return 0;
}
