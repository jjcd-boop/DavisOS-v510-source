#pragma once
#include "../Base/Types.hpp"
namespace Davis::Browser {
static constexpr u32 MaxHost=128,MaxPath=512,MaxHeader=2048,MaxLocation=384,MaxContentType=64;
struct Url { char host[MaxHost]; char path[MaxPath]; u16 port; bool https,valid; };
struct HttpResponse { u16 status; u32 headerBytes,bodyBytes,contentLength; bool complete,chunked,hasContentLength,connectionClose; char location[MaxLocation]; char contentType[MaxContentType]; };
bool ParseUrl(const char* text,Url& out);
u32 BuildHttpGet(const Url& url,char* out,u32 capacity);
bool ParseHttpResponse(const char* data,u32 bytes,HttpResponse& out);
bool DecodeChunked(const char* input,u32 inputBytes,char* output,u32 capacity,u32& outputBytes,bool& complete);
bool ResolveUrl(const Url& base,const char* reference,char* out,u32 capacity);
}
