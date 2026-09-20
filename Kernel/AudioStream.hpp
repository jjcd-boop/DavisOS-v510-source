#pragma once
#include "../Base/Types.hpp"
#include "AudioMedia.hpp"
namespace Davis::AudioStream {
enum class PlaybackState:u8{Stopped,Playing,Paused,Ended};
struct Player{AudioMedia::Decoder decoder;u64 streamId;u32 sourceRate;u64 phase;PlaybackState state;bool eof;};
extern Player player;
bool Start(const u8* data,u32 bytes); bool StartWav(const u8* data,u32 bytes);void Stop();void Pause();void Resume();bool Seek(u64 frame);u64 Position();PlaybackState State();void Pump();
}
