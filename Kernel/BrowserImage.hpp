#pragma once
#include "../Base/Types.hpp"
#include "Media.hpp"
namespace Davis::BrowserImage {
static constexpr u32 MaxImages=4, MaxEncoded=65536, MaxWidth=1920, MaxHeight=1080, MaxPixelBytes=MaxWidth*MaxHeight*4;
using Format=Media::ImageFormat;
struct Image{char url[384];Format format;u16 width,height;u32 encodedBytes,pixelCount;u8 encoded[MaxEncoded];u32* pixels;u64 pixelPhysical,pixelPages;bool valid,decoded,truncated;};
struct State{Image images[MaxImages];u32 count;u64 detected,decoded,failed,allocFailures;};
extern State state;
void Init(); Format Detect(const u8*,u32); bool Store(const char* url,const u8* data,u32 bytes); const Image* Find(const char* url);
bool AllocatePixels(Image&,u32 width,u32 height); void Release(Image&);
}
