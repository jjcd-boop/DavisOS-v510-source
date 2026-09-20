#pragma once
#include "../Base/Types.hpp"
#include "UserMode.hpp"
#include "Idt.hpp"
namespace Davis::PlatformState {
struct Snapshot { u16 cs,ss,ds,tr; u64 cr3; bool selectorsOwned; bool tssLoaded; bool pagingActive; };
Snapshot Capture(const UserMode::GdtState&);
bool ValidateOwned(const Snapshot&,const UserMode::GdtState&);
}
