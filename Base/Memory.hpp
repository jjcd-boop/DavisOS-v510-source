#pragma once
#include "Types.hpp"
namespace Davis {
inline void* MemSet(void*p,u8 v,usize n){auto*d=(u8*)p;while(n--)*d++=v;return p;}
inline void* MemCopy(void*d,const void*s,usize n){auto*a=(u8*)d;auto*b=(const u8*)s;while(n--)*a++=*b++;return d;}
inline usize StrLen(const char*s){usize n=0;if(s)while(s[n])++n;return n;}
}
