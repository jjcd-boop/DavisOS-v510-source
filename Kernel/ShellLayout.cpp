#include "ShellLayout.hpp"
namespace Davis::ShellLayout {
Layout Compute(const Graphics::Surface&s){
 int W=(int)s.fb.width,H=(int)s.fb.height,m=W>=1600?18:10,top=H>=900?58:46,task=H>=900?70:58,rail=W>=1200?190:150;
 return {{0,0,W,top},{0,H-task,W,task},{m,H-task+8,W>=1200?150:120,task-16},
 {W-rail-m,top+m,rail,H-top-task-2*m},{m,top+m,W-rail-3*m,H-top-task-2*m},m};
}
bool Hit(const Rect&r,int x,int y){return x>=r.x&&x<r.x+r.w&&y>=r.y&&y<r.y+r.h;}
}
