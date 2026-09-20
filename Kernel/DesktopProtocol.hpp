#pragma once
#include "../Base/Types.hpp"
namespace Davis::DesktopProtocol {
static constexpr u16 Major=1, Minor=1;
enum Type:u32 {
 Hello=0x44010001, CreateWindow=0x44010002, CloseWindow=0x44010003,
 HelloReply=0x44018001, CreateWindowReply=0x44018002, CloseWindowReply=0x44018003,
 FocusEvent=0x44020001, PointerEvent=0x44020002, KeyEvent=0x44020003, CloseEvent=0x44020004, InputInject=0x44030001
};
enum PointerFlags:u32 { PointerLeft=1u<<0, PointerPressed=1u<<1, PointerReleased=1u<<2 };
#pragma pack(push,1)
struct Header { u16 major,minor; u32 bytes; u64 replyHandle; u64 requestId; };
struct HelloRequest { Header h; };
struct HelloResponse { Header h; u32 status; u32 reserved; };
struct CreateWindowRequest { Header h; i32 x,y; u32 width,height; };
struct CreateWindowResponse { Header h; u32 status; u32 reserved; u64 windowHandle; };
struct CloseWindowRequest { Header h; u64 windowHandle; };
struct CloseWindowResponse { Header h; u32 status; u32 reserved; };
struct EventHeader { u16 major,minor; u32 bytes; u64 windowHandle; u64 sequence; };
struct Focus { EventHeader h; u32 focused; u32 reserved; };
struct Pointer { EventHeader h; i32 x,y; i32 dx,dy; u32 flags; u32 reserved; };
struct Key { EventHeader h; u32 codepoint; u32 flags; };
struct Close { EventHeader h; u32 reason; u32 reserved; };
struct InputInjection { Header h; u8 protocol,length; u16 reserved; u8 data[64]; };
#pragma pack(pop)
}
