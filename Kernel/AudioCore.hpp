#pragma once
#include "../Base/Types.hpp"
namespace Davis::Audio {
static constexpr u32 MaxStreams=8;
static constexpr u32 FramesPerBuffer=256;
struct Format{u32 sampleRate;u16 channels;u16 bitsPerSample;};
struct Stream{u64 id;Format format;i16 samples[FramesPerBuffer*2];u32 readFrame,writeFrame,queuedFrames;u32 volume;bool active,muted;};
struct State{Stream streams[MaxStreams];i16 mix[FramesPerBuffer*2];u64 nextId,pumps,framesMixed,underruns;u32 masterVolume;bool muted,ready;};
extern State state;
void Init();
u64 CreateStream(Format format);
bool DestroyStream(u64 id);
u32 QueueStereo16(u64 id,const i16* interleaved,u32 frames);
void SetMasterVolume(u32 percent);void SetMute(bool mute);
u32 Mix(i16* output,u32 frames);
void PumpOnce();
}
