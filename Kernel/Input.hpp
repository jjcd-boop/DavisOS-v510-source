#pragma once
#include "../Base/Types.hpp"
namespace Davis::Input {
struct State{ i64 mouseX,mouseY; bool left,prevLeft,right,middle; bool pressLatched,releaseLatched; bool shift,ctrl,alt,extended; bool systemKeyPressed; u8 mousePacket[3]; u8 mouseIndex; u64 keyEvents,mousePackets; };
void Init(State&,u32 width,u32 height);
void Poll(State&,u32 width,u32 height);
bool Clicked(const State&);
bool Released(const State&);
char PopChar();
}
