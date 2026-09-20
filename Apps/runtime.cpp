extern "C" void* memset(void*d,int v,unsigned long n){auto*p=(unsigned char*)d;while(n--)*p++=(unsigned char)v;return d;}
extern "C" void* memcpy(void*d,const void*s,unsigned long n){auto*a=(unsigned char*)d;auto*b=(const unsigned char*)s;while(n--)*a++=*b++;return d;}
