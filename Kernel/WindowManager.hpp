#pragma once
#include "../Base/Types.hpp"
namespace Davis::WindowManager {
enum WindowId:u32 { Files=0, Writer=1, Terminal=2, Settings=3, Browser=4, MathLab=5, PlaneShooter=6, Solitaire=7, Chess=8, Checkers=9, MediaPlayer=10, PhotoViewer=11, Count=12 };
struct Rect { i64 x,y,w,h; };
struct Window { Rect bounds,restore; bool visible,minimized,maximized; u32 z; };
struct State { Window windows[Count]; u32 nextZ; i32 dragging; i32 resizing; i64 dragDx,dragDy; };
void Init(State&,u32,u32); void Open(State&,WindowId); void Close(State&,WindowId); void Minimize(State&,WindowId);
void ToggleMaximize(State&,WindowId,u32,u32); void Focus(State&,WindowId); i32 TopAt(const State&,i64,i64);
void PointerDown(State&,i64,i64,u32,u32); void PointerMove(State&,i64,i64,u32,u32); void PointerUp(State&);
bool Hit(const Rect&,i64,i64);
}
