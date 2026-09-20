#pragma once
#include "../Base/Types.hpp"
namespace Davis::BrowserLayout {
static constexpr u32 MaxBoxes=256;
struct Box{u32 nodeIndex,textOffset,textBytes; i32 x,y,w,h; bool link;u16 linkIndex;bool valid;};
struct Layout{Box boxes[MaxBoxes];u32 count; i32 contentHeight,scrollY;u32 viewportW,viewportH;};
extern Layout state;
void Build(u32 viewportW,u32 viewportH);void Scroll(i32 delta);i32 HitLink(i32 x,i32 y);
}
