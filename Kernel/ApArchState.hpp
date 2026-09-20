#pragma once
#include "../Base/Types.hpp"
namespace Davis::ApArchState {
enum class Stage:u8 { Reset, RealMode, ProtectedMode, PagingPrepared, LongMode, PerCpuReady, Online, Failed };
struct State { u32 apicId; u32 logicalId; u64 cr3,gdt,tss,stackTop,gsBase,entryPoint; Stage stage; u64 transitions; };
bool Prepare(State&,u32 apicId,u32 logicalId,u64 cr3,u64 gdt,u64 tss,u64 stackTop,u64 gsBase,u64 entryPoint);
bool EnterRealMode(State&); bool EnterProtectedMode(State&); bool PreparePaging(State&); bool EnterLongMode(State&); bool InstallPerCpu(State&); bool MarkOnline(State&); bool Fail(State&); bool Validate(const State&);
}
