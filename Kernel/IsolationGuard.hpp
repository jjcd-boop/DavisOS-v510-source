#pragma once
#include "../Base/Types.hpp"
#include "Process.hpp"
namespace Davis::IsolationGuard {
enum class FaultKind:u32{None=0,Exit=1,UserFault=2,Watchdog=3,Protocol=4};
enum class Status:u32{Empty=0,Healthy=1,Degraded=2,Quarantined=3,TearingDown=4,Dead=5};
struct Cell{u64 processId;SecurityDomain::Kind domain;Status status;u32 faults;u32 teardownGeneration;FaultKind lastFault;u64 lastVector;bool active;};
static constexpr usize MaxCells=Process::MaxProcesses;
struct State{Cell cells[MaxCells];bool ready;};
extern State state;
void Init();
void Admit(const Process::Image&);
void RecordFault(const Process::Image&,FaultKind,u64 detail);
bool BeginTeardown(u64 processId);
void FinishTeardown(u64 processId);
bool MayUsePrivilegedDriverPath(const Process::Image&);
const Cell* Find(u64 processId);
}
