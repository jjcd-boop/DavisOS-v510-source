#include "BrowserLayout.hpp"
#include "BrowserDocument.hpp"
#include "BrowserStyle.hpp"
namespace Davis::BrowserLayout { Layout state{};
void Build(u32 vw,u32 vh){i32 old=state.scrollY;state={};state.viewportW=vw;state.viewportH=vh;state.scrollY=old;i32 y=8;for(u32 i=0;i<BrowserDocument::state.nodeCount&&state.count<MaxBoxes;i++){auto&n=BrowserDocument::state.nodes[i];if(!n.valid||!n.textBytes)continue;auto s=BrowserStyle::Compute(n);i32 available=(i32)vw-16-s.paddingLeft-s.paddingRight-2*s.borderWidth;if(s.width>0&&s.width<available)available=s.width;if(available<64)available=64;u32 chars=(u32)available/(8*s.scale);if(chars<8)chars=8;u32 lines=(n.textBytes+chars-1)/chars;if(!lines)lines=1;y+=s.marginTop;Box&b=state.boxes[state.count++];i32 natural=(i32)(lines*16*s.scale)+2*s.borderWidth;i32 h=s.height>natural?s.height:natural;b={i,n.textOffset,n.textBytes,8+s.paddingLeft+s.borderWidth,y,available,h,n.type==BrowserDocument::NodeType::Link,n.linkIndex,true};y+=b.h+s.marginBottom;}state.contentHeight=y+8;i32 max=state.contentHeight>(i32)vh?state.contentHeight-(i32)vh:0;if(state.scrollY>max)state.scrollY=max;if(state.scrollY<0)state.scrollY=0;}
void Scroll(i32 d){state.scrollY+=d;i32 max=state.contentHeight>(i32)state.viewportH?state.contentHeight-(i32)state.viewportH:0;if(state.scrollY<0)state.scrollY=0;if(state.scrollY>max)state.scrollY=max;}
i32 HitLink(i32 x,i32 y){y+=state.scrollY;for(u32 i=0;i<state.count;i++){auto&b=state.boxes[i];if(b.link&&x>=b.x&&x<b.x+b.w&&y>=b.y&&y<b.y+b.h)return b.linkIndex;}return -1;}
}
