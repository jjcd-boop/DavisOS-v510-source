#include "AudioStream.hpp"
#include "AudioCore.hpp"
namespace Davis::AudioStream {Player player{};static constexpr u32 OutRate=48000;
void Stop(){if(player.streamId)Audio::DestroyStream(player.streamId);player={};player.state=PlaybackState::Stopped;}
bool Start(const u8*d,u32 n){Stop();if(!AudioMedia::Open(player.decoder,d,n))return false;player.sourceRate=player.decoder.info.sampleRate;player.streamId=Audio::CreateStream({OutRate,2,16});if(!player.streamId){player={};return false;}player.state=PlaybackState::Playing;return true;}
bool StartWav(const u8*d,u32 n){AudioMedia::Info i{};if(!AudioMedia::Probe(d,n,i)||i.codec!=AudioMedia::Codec::WavPcm)return false;return Start(d,n);}
void Pause(){if(player.state==PlaybackState::Playing)player.state=PlaybackState::Paused;}
void Resume(){if(player.state==PlaybackState::Paused)player.state=PlaybackState::Playing;}
bool Seek(u64 frame){if(!player.decoder.open||frame>player.decoder.info.totalFrames)return false;if(!AudioMedia::SeekFrame(player.decoder,frame))return false;player.phase=frame*(u64)OutRate;player.eof=false;if(player.state==PlaybackState::Ended)player.state=PlaybackState::Paused;return true;}
u64 Position(){return player.sourceRate?player.phase/OutRate:0;} PlaybackState State(){return player.state;}
static i16 lerp(i16 a,i16 b,u32 frac){i32 d=(i32)b-(i32)a;return (i16)((i32)a+(i32)(((i64)d*frac)/OutRate));}
void Pump(){if(player.state!=PlaybackState::Playing||player.eof)return;auto&s=Audio::state;u32 free=Audio::FramesPerBuffer;for(auto&x:s.streams)if(x.active&&x.id==player.streamId){free=Audio::FramesPerBuffer-x.queuedFrames;break;}if(!free)return;static i16 src[Audio::FramesPerBuffer*2],out[Audio::FramesPerBuffer*2];
// FLAC is deliberately streamed sequentially. WAV keeps the interpolating random-access path.
if(player.decoder.info.codec==AudioMedia::Codec::Flac){u32 want=(u32)(((u64)free*player.sourceRate+OutRate-1)/OutRate);if(want>Audio::FramesPerBuffer)want=Audio::FramesPerBuffer;u32 got=AudioMedia::ReadStereo16(player.decoder,src,want);if(!got){player.eof=true;player.state=PlaybackState::Ended;return;}u32 produced=0;u64 step=((u64)player.sourceRate<<32)/OutRate;u64 ph=0;while(produced<free){u32 ix=(u32)(ph>>32);if(ix>=got)break;out[produced*2]=src[ix*2];out[produced*2+1]=src[ix*2+1];produced++;ph+=step;}player.phase=AudioMedia::TellFrame(player.decoder)*(u64)OutRate;if(produced)Audio::QueueStereo16(player.streamId,out,produced);return;}
static i16 pair[4];u32 produced=0;while(produced<free){u64 base=player.phase/OutRate;u32 frac=(u32)(player.phase%OutRate);if(base>=player.decoder.info.totalFrames){player.eof=true;player.state=PlaybackState::Ended;break;}if(!AudioMedia::SeekFrame(player.decoder,base))break;u32 want=(base+1<player.decoder.info.totalFrames)?2:1;u32 got=AudioMedia::ReadStereo16(player.decoder,pair,want);if(!got){player.eof=true;player.state=PlaybackState::Ended;break;}i16 l=pair[0],r=pair[1];if(got>1){l=lerp(pair[0],pair[2],frac);r=lerp(pair[1],pair[3],frac);}out[produced*2]=l;out[produced*2+1]=r;produced++;player.phase+=player.sourceRate;}if(produced)Audio::QueueStereo16(player.streamId,out,produced);}
}
