#pragma once
#include "../Base/Types.hpp"
#include "Process.hpp"
namespace Davis::DriverManager {
enum class State:u32 { Empty=0,Registered,Starting,Running,Failed,Quarantined };
struct Driver { u64 processId; u64 deviceId; State state; u64 heartbeat; u32 failures; bool active; };
static constexpr usize MaxDrivers=32;
struct Assignment { u64 processId; u64 deviceId; bool active; };
struct Table { Driver drivers[MaxDrivers]; Assignment assignments[MaxDrivers]; bool ready; };
extern Table table;
void Init();
bool AssignTrusted(u64 processId,u64 deviceId);
u64 AssignedDevice(u64 processId);
bool Register(Process::Image&,u64 deviceId);
bool Heartbeat(const Process::Image&,u64 deviceId);
void MarkFailed(u64 processId);
void RevokeProcess(u64 processId);
const Driver* FindByDevice(u64 deviceId);
}
