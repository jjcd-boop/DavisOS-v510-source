#include "MediaLibrary.hpp"
#include "AudioMedia.hpp"
#include "BrowserImage.hpp"
namespace Davis::MediaLibrary {State state{};
static bool starts(const char*s,const char*p){while(*p){char a=*s++,b=*p++;if(a>='a'&&a<='z')a-=32;if(b>='a'&&b<='z')b-=32;if(a!=b)return false;}return true;}
static bool ext(const char*s,const char*e){u32 n=0,m=0;while(s[n])n++;while(e[m])m++;if(n<m)return false;for(u32 i=0;i<m;i++){char a=s[n-m+i],b=e[i];if(a>='A'&&a<='Z')a+=32;if(b>='A'&&b<='Z')b+=32;if(a!=b)return false;}return true;}
static bool playable(const BootMedia&m){if(m.kind!=MediaAudio)return false;return ext(m.path,".wav")||ext(m.path,".flac")||ext(m.path,".mp3")||ext(m.path,".ogg")||ext(m.path,".mid")||ext(m.path,".midi");}
static void rebuild(){state.audioCount=state.imageCount=0;if(!state.boot)return;for(u32 i=0;i<state.boot->mediaCount&&i<MaxBootMedia;i++){auto&m=state.boot->media[i];bool aa=state.searchAll||starts(m.path,"AUDIO/"),pp=state.searchAll||starts(m.path,"PICTURES/");if(aa&&playable(m)&&state.audioCount<MaxBootMedia)state.audio[state.audioCount++]=i;if(pp&&m.kind==MediaImage&&state.imageCount<MaxBootMedia)state.images[state.imageCount++]=i;}}
void Init(const BootInfo&b){state={};state.boot=&b;rebuild();state.status[0]=0;}void SetSearchAll(bool v){state.searchAll=v;rebuild();}
const BootMedia* AudioAt(u32 i){return state.boot&&i<state.audioCount?&state.boot->media[state.audio[i]]:nullptr;}const BootMedia* ImageAt(u32 i){return state.boot&&i<state.imageCount?&state.boot->media[state.images[i]]:nullptr;}
bool Play(u32 i){auto*m=AudioAt(i);if(!m)return false;state.selectedAudio=i;bool ok=AudioStream::Start((const u8*)m->base,(u32)m->bytes);const char*t=ok?"PLAYING":"DECODER/AUDIO START FAILED";u32 z=0;while(t[z]&&z<63){state.status[z]=t[z];z++;}state.status[z]=0;return ok;}
void Stop(){AudioStream::Stop();const char*t="STOPPED";u32 z=0;while(t[z]){state.status[z]=t[z];z++;}state.status[z]=0;}
bool Show(u32 i){auto*m=ImageAt(i);if(!m)return false;state.selectedImage=i;return BrowserImage::Store(m->path,(const u8*)m->base,(u32)m->bytes);}
}
