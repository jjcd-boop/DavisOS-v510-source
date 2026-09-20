#include "DxeCompositor.hpp"
#include "DxeServices.hpp"
#include "GraphicsObjectManager.hpp"
namespace Davis::DxeCompositor {
static void u64text(char* b,u64 v){char t[24];u32 n=0;if(!v)t[n++]='0';while(v&&n<23){t[n++]=(char)('0'+v%10);v/=10;}u32 j=0;while(n)b[j++]=t[--n];b[j]=0;}
static void Blit(Graphics::Surface&g,i64 x,i64 y,const DxeServices::WindowSurface&s,i64 maxW,i64 maxH){
 i64 w=s.width< (u64)maxW?s.width:maxW,h=s.height<(u64)maxH?s.height:maxH;if(w<=0||h<=0)return;
 const auto*o=GraphicsObjectManager::FindAny(s.graphicsObjectId);if(!o||!o->physical)return;const auto*src=(const u32*)(uptr)o->physical;auto*dst=(u32*)g.fb.base;for(i64 yy=0;yy<h;yy++){i64 dy=y+yy;if(dy<0||(u64)dy>=g.fb.height)continue;for(i64 xx=0;xx<w;xx++){i64 dx=x+xx;if(dx<0||(u64)dx>=g.fb.width)continue;dst[(usize)dy*g.fb.pixelsPerScanLine+(usize)dx]=src[(usize)yy*o->stride+(usize)xx];}}
}
Stats Draw(Graphics::Surface&g,u32 accent,u32 panel,u32 text,u32 muted){Stats s{};u64 lastZ=0;for(usize pass=0;pass<DxeServices::MaxWindows;pass++){DxeServices::Window*wp=nullptr;for(auto&candidate:DxeServices::state.windows)if(candidate.active&&candidate.z>lastZ&&(!wp||candidate.z<wp->z))wp=&candidate;if(!wp)break;auto&w=*wp;lastZ=w.z;s.visibleWindows++;s.totalPresents+=w.presents;if(w.presents)s.presentedWindows++;i64 x=w.x,y=w.y,ww=w.w,hh=w.h;if(x<0)x=0;if(y<56)y=56;if(x+ww>(i64)g.fb.width)ww=g.fb.width>x?g.fb.width-x:0;if(y+hh+32>(i64)g.fb.height-56)hh=g.fb.height-56>y+32?g.fb.height-56-y-32:0;if(ww<64||hh<48)continue;Graphics::Rect(g,x+4,y+5,ww,hh+32,0x00101820);Graphics::Rect(g,x,y,ww,hh+32,panel);Graphics::Frame(g,x,y,ww,hh+32,accent,2);Graphics::Rect(g,x,y,ww,32,accent);Graphics::Text(g,x+10,y+10,"DXE APPLICATION",text,1);Graphics::Text(g,x+ww-20,y+10,"X",text,1);Graphics::Rect(g,x+ww-10,y+hh+22,8,8,accent);
 const auto*sf=DxeServices::FindWindowSurface(w.owner,w.id);if(sf&&w.presents)Blit(g,x,y+32,*sf,ww,hh);else{Graphics::Text(g,x+14,y+52,"WAITING FOR PIXELS",muted,1);Graphics::Text(g,x+14,y+76,"OWNER PID",muted,1);char b[24];u64text(b,w.owner);Graphics::Text(g,x+104,y+76,b,text,1);}
}return s;}
}
