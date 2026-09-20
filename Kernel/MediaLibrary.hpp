#pragma once
#include "BootInfo.hpp"
#include "AudioStream.hpp"
namespace Davis::MediaLibrary {
struct State{const BootInfo* boot;u32 audio[MaxBootMedia],images[MaxBootMedia];u32 audioCount,imageCount;u32 selectedAudio,selectedImage;bool searchAll;char status[64];};
extern State state;void Init(const BootInfo&);void SetSearchAll(bool);bool Play(u32 index);void Stop();bool Show(u32 index);const BootMedia* AudioAt(u32);const BootMedia* ImageAt(u32);
}
