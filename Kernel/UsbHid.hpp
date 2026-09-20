#pragma once
#include "../Base/Types.hpp"
namespace Davis::UsbHid {
struct Keyboard { bool shift; bool ctrl; bool alt; u8 previous[6]; };
struct Mouse { int dx,dy,wheel; bool left,right,middle; };
void Init(Keyboard&);
char DecodeBootKeyboard(Keyboard&,const u8 report[8]);
void DecodeBootMouse(Mouse&,const u8*report,usize bytes);
}
