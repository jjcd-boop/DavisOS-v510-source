#include "AudioCore.hpp"
namespace Davis::Audio {
State state{};
static Stream* find(u64 id){for(auto&s:state.streams)if(s.active&&s.id==id)return &s;return nullptr;}
static i16 clamp(i32 v){if(v>32767)return 32767;if(v<-32768)return -32768;return (i16)v;}
void Init(){state={};state.nextId=1;state.masterVolume=100;state.ready=true;}
u64 CreateStream(Format f){if(!state.ready||f.sampleRate!=48000||f.channels!=2||f.bitsPerSample!=16)return 0;for(auto&s:state.streams)if(!s.active){s={};s.active=true;s.id=state.nextId++;s.format=f;s.volume=100;return s.id;}return 0;}
bool DestroyStream(u64 id){auto*s=find(id);if(!s)return false;*s={};return true;}
u32 QueueStereo16(u64 id,const i16*in,u32 frames){auto*s=find(id);if(!s||!in)return 0;u32 n=0;while(n<frames&&s->queuedFrames<FramesPerBuffer){u32 w=s->writeFrame; s->samples[w*2]=in[n*2];s->samples[w*2+1]=in[n*2+1];s->writeFrame=(w+1)%FramesPerBuffer;s->queuedFrames++;n++;}return n;}
void SetMasterVolume(u32 p){state.masterVolume=p>100?100:p;}void SetMute(bool m){state.muted=m;}
u32 Mix(i16*out,u32 frames){if(!state.ready||!out)return 0;if(frames>FramesPerBuffer)frames=FramesPerBuffer;for(u32 f=0;f<frames;f++){i32 l=0,r=0;bool any=false;for(auto&s:state.streams)if(s.active&&s.queuedFrames){i32 sl=s.samples[s.readFrame*2],sr=s.samples[s.readFrame*2+1];s.readFrame=(s.readFrame+1)%FramesPerBuffer;s.queuedFrames--;if(!s.muted){l+=(sl*(i32)s.volume)/100;r+=(sr*(i32)s.volume)/100;}any=true;}if(!any)state.underruns++;if(state.muted)l=r=0;else{l=(l*(i32)state.masterVolume)/100;r=(r*(i32)state.masterVolume)/100;}out[f*2]=clamp(l);out[f*2+1]=clamp(r);}state.framesMixed+=frames;return frames;}
void PumpOnce(){if(!state.ready)return;Mix(state.mix,FramesPerBuffer);state.pumps++;}
}
