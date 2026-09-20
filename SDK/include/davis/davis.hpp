#pragma once
#include <stdint.h>
#include <stddef.h>
namespace DavisSDK {
using Handle=uint64_t;
struct Result { int64_t value; uint64_t error; constexpr explicit operator bool()const{return error==0;} };
enum Syscall:uint64_t { Exit=0,Log=1,Yield=2,CreateWindow=16,PresentWindow=17,OpenFile=32,ReadFile=33,WriteFile=34,CloseFile=35,ServiceLookup=48,IpcCreate=49,IpcSend=50,IpcReceive=51,IpcClose=52 };
enum Service:uint64_t { DesktopCompositor=1,FileService=2 };
enum IpcRights:uint64_t { IpcRightSend=1ull<<0,IpcRightReceive=1ull<<1 };
struct IpcMessage { uint64_t sender; uint32_t type; uint32_t bytes; uint8_t payload[192]; };
enum OpenFlags:uint64_t { OpenRead=1ull<<0,OpenWrite=1ull<<1,OpenCreate=1ull<<2,OpenTruncate=1ull<<3 };
#if defined(__x86_64__)
static inline Result Call(uint64_t n,uint64_t a0=0,uint64_t a1=0,uint64_t a2=0,uint64_t a3=0,uint64_t a4=0,uint64_t a5=0){
 register uint64_t rax asm("rax")=n; register uint64_t rdi asm("rdi")=a0; register uint64_t rsi asm("rsi")=a1; register uint64_t rdx asm("rdx")=a2; register uint64_t r10 asm("r10")=a3; register uint64_t r8 asm("r8")=a4; register uint64_t r9 asm("r9")=a5;
 asm volatile("int $0x80":"+a"(rax),"+d"(rdx):"D"(rdi),"S"(rsi),"r"(r10),"r"(r8),"r"(r9):"rcx","r11","memory","cc"); return {(int64_t)rax,rdx}; }
#else
static inline Result Call(uint64_t,uint64_t=0,uint64_t=0,uint64_t=0,uint64_t=0,uint64_t=0,uint64_t=0){return {-1,95};}
#endif
[[noreturn]] static inline void ProcessExit(int code){(void)Call(Exit,(uint64_t)code);for(;;){}}
static inline Result ProcessYield(){return Call(Yield);} static inline Result LogWrite(const char* p,size_t n){return Call(Log,(uint64_t)p,n);}
static inline Result WindowCreateAt(int64_t x,int64_t y,uint64_t w,uint64_t h){return Call(CreateWindow,(uint64_t)x,(uint64_t)y,w,h);} static inline Result WindowCreate(uint64_t w,uint64_t h){return WindowCreateAt(96,96,w,h);} static inline Result WindowPresent(Handle h,const void* p,size_t n){return Call(PresentWindow,h,(uint64_t)p,n);}
static inline Result FileOpen(const char* path,uint64_t flags){return Call(OpenFile,(uint64_t)path,flags);} static inline Result FileRead(Handle h,void* p,size_t n){return Call(ReadFile,h,(uint64_t)p,n);} static inline Result FileWrite(Handle h,const void* p,size_t n){return Call(WriteFile,h,(uint64_t)p,n);} static inline Result FileClose(Handle h){return Call(CloseFile,h);}
static inline Result LookupService(uint64_t service){return Call(ServiceLookup,service);}
static inline Result CreateIpc(uint64_t rights){return Call(IpcCreate,rights);}
static inline Result IpcSendMessage(Handle h,uint32_t type,const void* p,size_t n){return Call(IpcSend,h,type,(uint64_t)p,n);}
static inline Result IpcReceiveMessage(Handle h,IpcMessage* out){return Call(IpcReceive,h,(uint64_t)out);}
static inline Result IpcCloseHandle(Handle h){return Call(IpcClose,h);}
}

namespace DavisDesktopV1 {
static constexpr uint16_t Major=1,Minor=0;
enum Type:uint32_t { Hello=0x44010001,CreateWindow=0x44010002,CloseWindow=0x44010003,HelloReply=0x44018001,CreateWindowReply=0x44018002,CloseWindowReply=0x44018003 };
#pragma pack(push,1)
struct Header{uint16_t major,minor;uint32_t bytes;uint64_t replyHandle;uint64_t requestId;};
struct HelloRequest{Header h;}; struct HelloResponse{Header h;uint32_t status,reserved;};
struct CreateWindowRequest{Header h;int32_t x,y;uint32_t width,height;}; struct CreateWindowResponse{Header h;uint32_t status,reserved;uint64_t windowHandle;};
struct CloseWindowRequest{Header h;uint64_t windowHandle;}; struct CloseWindowResponse{Header h;uint32_t status,reserved;};
#pragma pack(pop)
}
