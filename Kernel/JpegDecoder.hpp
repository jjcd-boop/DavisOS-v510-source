#pragma once
#include "../Base/Types.hpp"
namespace Davis::BrowserImage { struct Image; }
namespace Davis::JpegDecoder { bool Decode(const u8* data,u32 bytes,BrowserImage::Image& out); }
