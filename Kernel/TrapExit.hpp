#pragma once
#include "../Base/Types.hpp"
#include "Process.hpp"
namespace Davis::TrapExit {
enum class Reason:u32 { None, ProcessExit, UserFault, CooperativeYield, SystemSlice };
struct State { Process::Image* current; Reason reason; int code; u64 vector; bool requested; };
extern State state;
void Arm(Process::Image*);
void RequestExit(int code);
void RequestFault(u64 vector);
void RequestYield();
void RequestSystemSlice();
bool ShouldAbandonUser();
void Clear();
extern "C" [[noreturn]] void DavisTrapAbandonToKernel();
}
