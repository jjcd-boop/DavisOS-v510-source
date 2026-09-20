#pragma once
#include "../Base/Types.hpp"
#include "UserMode.hpp"
#include "Idt.hpp"
#include "StackGuard.hpp"
namespace Davis::EmergencyIst {
struct State { StackGuard::Stack doubleFault,nmi,machineCheck; bool installed; };
extern State state;
bool Prepare(UserMode::GdtState&,Idt::Table&,u16 kernelCode);
bool MapInto(Paging::Space&,Memory::PageAllocator&);
bool Validate(const UserMode::GdtState&,const Idt::Table&,u16 kernelCode);
bool CanariesIntact();
extern "C" void DavisDoubleFault(); extern "C" [[noreturn]] void DavisDoubleFaultDispatch(u64* frame);
}
