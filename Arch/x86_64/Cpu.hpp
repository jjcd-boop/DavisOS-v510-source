#pragma once
#include "../../Base/Types.hpp"
namespace Davis::Cpu {
inline void Pause(){__asm__ __volatile__("pause");}
inline void Halt(){__asm__ __volatile__("hlt");}
inline void DisableInterrupts(){__asm__ __volatile__("cli");}
inline void EnableInterrupts(){__asm__ __volatile__("sti");}
inline void Fence(){__asm__ __volatile__("":::"memory");}
[[noreturn]] inline void HaltForever(){DisableInterrupts();for(;;)Halt();}
}
