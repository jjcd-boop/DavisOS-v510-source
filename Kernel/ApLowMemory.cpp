#include "ApLowMemory.hpp"
extern "C" const unsigned char DavisApTrampolineBlobStart[];
extern "C" const unsigned char DavisApTrampolineBlobEnd[];
namespace Davis::ApLowMemory {
static void copy(void*d,const void*s,u32 n){auto*a=(u8*)d;auto*b=(const u8*)s;for(u32 i=0;i<n;i++)a[i]=b[i];}
u32 TrampolineBytes(){auto n=(u64)(DavisApTrampolineBlobEnd-DavisApTrampolineBlobStart);return n<=0xffffffffu?(u32)n:0;}
bool Build(Image&i,u64 p,const ApTrampoline::Handoff&h){i={};u32 n=TrampolineBytes();if(p<0x1000||p>=0x100000||(p&0xfff)||!ApTrampoline::Validate(h)||!ApTrampoline::ValidatePrePaging(h)||!n||n>HandoffOffset||HandoffOffset+sizeof(h)>ImageBytes)return false;i.physical=p;copy(i.bytes,DavisApTrampolineBlobStart,n);copy(i.bytes+HandoffOffset,&h,sizeof(h));i.ready=true;return true;}
const ApTrampoline::Handoff*GetHandoff(const Image&i){return(const ApTrampoline::Handoff*)(i.bytes+HandoffOffset);}
bool Validate(const Image&i){u32 n=TrampolineBytes();if(!i.ready||i.physical<0x1000||i.physical>=0x100000||(i.physical&0xfff)||!n||n>HandoffOffset)return false;for(u32 x=0;x<n;x++)if(i.bytes[x]!=DavisApTrampolineBlobStart[x])return false;return ApTrampoline::Validate(*GetHandoff(i));}
}
