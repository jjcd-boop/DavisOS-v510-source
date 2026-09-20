#pragma once
#include "Graphics.hpp"
namespace Davis::ShellLayout {
struct Rect{int x,y,w,h;};
struct Layout{Rect topbar,taskbar,start,rightRail,desktop;int margin;};
Layout Compute(const Graphics::Surface&);
bool Hit(const Rect&,int,int);
}
