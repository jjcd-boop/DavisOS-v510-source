#include "WindowManager.hpp"
namespace Davis::WindowManager {
bool Hit(const Rect&r,i64 x,i64 y){return x>=r.x&&y>=r.y&&x<r.x+r.w&&y<r.y+r.h;}
static Rect R(i64 x,i64 y,i64 w,i64 h){return{x,y,w,h};}
void Init(State&s,u32 W,u32 H){s={};s.nextZ=10;s.dragging=-1;s.resizing=-1;s.windows[Files].bounds=R(W/8,H/7,W*55/100,H*55/100);s.windows[Writer].bounds=R(W/7,H/6,W*58/100,H*58/100);s.windows[Terminal].bounds=R(W/5,H/5,W*52/100,H*45/100);s.windows[Settings].bounds=R(W/4,H/7,W*58/100,H*56/100);s.windows[Browser].bounds=R(W/10,H/9,W*72/100,H*68/100);s.windows[MathLab].bounds=R(W/6,H/6,W*56/100,H*52/100);s.windows[PlaneShooter].bounds=R(W/6,H/7,W*58/100,H*62/100);s.windows[Solitaire].bounds=R(W/7,H/8,W*62/100,H*68/100);s.windows[Chess].bounds=R(W/5,H/9,W*52/100,H*72/100);s.windows[Checkers].bounds=R(W/5,H/9,W*52/100,H*72/100);s.windows[MediaPlayer].bounds=R(W/6,H/7,W*58/100,H*58/100);s.windows[PhotoViewer].bounds=R(W/7,H/8,W*64/100,H*66/100);for(u32 i=0;i<Count;i++){s.windows[i].restore=s.windows[i].bounds;s.windows[i].visible=false;s.windows[i].z=i+1;}s.windows[Settings].visible=false;}
void Focus(State&s,WindowId id){s.windows[id].z=s.nextZ++;}
void Open(State&s,WindowId id){auto&w=s.windows[id];w.visible=true;w.minimized=false;Focus(s,id);}
void Close(State&s,WindowId id){s.windows[id].visible=false;s.windows[id].minimized=false;}
void Minimize(State&s,WindowId id){s.windows[id].minimized=true;}
void ToggleMaximize(State&s,WindowId id,u32 W,u32 H){auto&w=s.windows[id];if(!w.maximized){w.restore=w.bounds;w.bounds=R(8,62,W-28,H-138);w.maximized=true;}else{w.bounds=w.restore;w.maximized=false;}Focus(s,id);}
i32 TopAt(const State&s,i64 x,i64 y){i32 best=-1;u32 z=0;for(u32 i=0;i<Count;i++){auto&w=s.windows[i];if(w.visible&&!w.minimized&&Hit(w.bounds,x,y)&&w.z>=z){z=w.z;best=(i32)i;}}return best;}
void PointerDown(State&s,i64 x,i64 y,u32 W,u32 H){i32 id=TopAt(s,x,y);if(id<0)return;auto&w=s.windows[id];Focus(s,(WindowId)id);i64 right=w.bounds.x+w.bounds.w; // title buttons
if(y<w.bounds.y+32&&x>=right-30){Close(s,(WindowId)id);return;}if(y<w.bounds.y+32&&x>=right-58){ToggleMaximize(s,(WindowId)id,W,H);return;}if(y<w.bounds.y+32&&x>=right-86){Minimize(s,(WindowId)id);return;}
if(!w.maximized&&x>=right-12&&y>=w.bounds.y+w.bounds.h-12){s.resizing=id;s.dragDx=right-x;s.dragDy=w.bounds.y+w.bounds.h-y;return;}
if(!w.maximized&&y<w.bounds.y+34){s.dragging=id;s.dragDx=x-w.bounds.x;s.dragDy=y-w.bounds.y;}}
void PointerMove(State&s,i64 x,i64 y,u32 W,u32 H){if(s.dragging>=0){auto&w=s.windows[s.dragging];w.bounds.x=x-s.dragDx;w.bounds.y=y-s.dragDy;if(w.bounds.x<0)w.bounds.x=0;if(w.bounds.y<54)w.bounds.y=54;if(w.bounds.x+w.bounds.w>(i64)W)w.bounds.x=W-w.bounds.w;if(w.bounds.y+w.bounds.h>(i64)H-64)w.bounds.y=H-64-w.bounds.h;}if(s.resizing>=0){auto&w=s.windows[s.resizing];i64 nw=x-w.bounds.x+s.dragDx,nh=y-w.bounds.y+s.dragDy;if(nw<300)nw=300;if(nh<180)nh=180;if(w.bounds.x+nw>(i64)W)nw=W-w.bounds.x;if(w.bounds.y+nh>(i64)H-64)nh=H-64-w.bounds.y;w.bounds.w=nw;w.bounds.h=nh;}}
void PointerUp(State&s){s.dragging=-1;s.resizing=-1;}
}
