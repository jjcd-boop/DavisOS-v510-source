#pragma once
#include "../Base/Types.hpp"
#include "Media.hpp"
namespace Davis::AudioMedia {
enum class Codec:u8{Unknown,WavPcm,Flac,Mp3};
struct Info{Codec codec;u32 sampleRate;u16 channels,bitsPerSample;u32 dataOffset,dataBytes;u64 totalFrames;};
static constexpr u32 MaxFlacBlock=4096;
struct Decoder{const u8* data;u32 bytes,pos;Info info;bool open;u64 framePos;u32 flacIndex,flacCount; i32 flacPcm[MaxFlacBlock*2];};
Codec Detect(const u8*,u32); bool Probe(const u8*,u32,Info&); bool Open(Decoder&,const u8*,u32);
u32 ReadStereo16(Decoder&,i16* out,u32 frames); bool SeekFrame(Decoder&,u64 frame); u64 TellFrame(const Decoder&);
}
