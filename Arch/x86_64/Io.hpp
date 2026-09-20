#pragma once
#include "../../Base/Types.hpp"
namespace Davis::Io {
inline u8 In8(u16 p){u8 v;__asm__ __volatile__("inb %1,%0":"=a"(v):"Nd"(p));return v;}
inline void Out8(u16 p,u8 v){__asm__ __volatile__("outb %0,%1"::"a"(v),"Nd"(p));}
inline u16 In16(u16 p){u16 v;__asm__ __volatile__("inw %1,%0":"=a"(v):"Nd"(p));return v;}
inline void Out16(u16 p,u16 v){__asm__ __volatile__("outw %0,%1"::"a"(v),"Nd"(p));}
inline u32 In32(u16 p){u32 v;__asm__ __volatile__("inl %1,%0":"=a"(v):"Nd"(p));return v;}
inline void Out32(u16 p,u32 v){__asm__ __volatile__("outl %0,%1"::"a"(v),"Nd"(p));}
}
