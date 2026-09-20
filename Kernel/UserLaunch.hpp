#pragma once
#include "../Base/Types.hpp"
#include "UserMode.hpp"
namespace Davis::UserLaunch {
struct Frame { u64 rip,rsp,rflags,cr3; u16 cs,ss; bool valid; };
bool Build(Frame&,u64 entry,u64 stackTop,u64 pml4,const UserMode::GdtState&);
u64 ReadCr3();
void WriteCr3(u64 physical);
}
