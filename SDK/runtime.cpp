#include <stddef.h>
extern "C" void* memcpy(void*d,const void*s,size_t n){auto*o=(unsigned char*)d;auto*i=(const unsigned char*)s;for(size_t k=0;k<n;k++)o[k]=i[k];return d;}
extern "C" void* memset(void*d,int v,size_t n){auto*o=(unsigned char*)d;for(size_t k=0;k<n;k++)o[k]=(unsigned char)v;return d;}
extern "C" void* memmove(void*d,const void*s,size_t n){auto*o=(unsigned char*)d;auto*i=(const unsigned char*)s;if(o<i)for(size_t k=0;k<n;k++)o[k]=i[k];else for(size_t k=n;k;k--)o[k-1]=i[k-1];return d;}
