#pragma once
#include "../Base/Types.hpp"
namespace Davis::Media {
enum class Kind:u8{Unknown,Image,Audio,Document};
enum class ImageFormat:u8{Unknown,Bmp,Png,Jpeg,Gif,WebP,Ico,Svg};
struct ByteStream { const u8* data; u32 bytes; u32 pos; bool Read(void* out,u32 n); bool Seek(u32 p); u32 Tell()const{return pos;} u32 Size()const{return bytes;} };
struct ImageInfo { ImageFormat format; u32 width,height; bool animated; };
ImageFormat DetectImage(const u8*,u32); bool ProbeImage(const u8*,u32,ImageInfo&);
}
