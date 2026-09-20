#pragma once
#include "BootInfo.hpp"
namespace Davis::Graphics {
struct Surface{Framebuffer fb;};
inline u32 RGB(u8 r,u8 g,u8 b){return ((u32)r<<16)|((u32)g<<8)|b;}
void Clear(Surface&,u32);void RoundRect(Surface&,i64,i64,i64,i64,u32,u32);
void Rect(Surface&,i64,i64,i64,i64,u32);void Frame(Surface&,i64,i64,i64,i64,u32,u32);
void Glyph(Surface&,i64,i64,char,u32,u32=1);void Text(Surface&,i64,i64,const char*,u32,u32=1);
}
