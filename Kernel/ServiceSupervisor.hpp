#pragma once
#include "../Base/Types.hpp"
namespace Davis::ServiceSupervisor {
enum class Service:u32 { Desktop,Input,Usb,Storage,Audio,Network,Clock,Writer,Calculator,Filesystem,Count };
enum class Health:u32 { Offline,Starting,Running,Degraded,Failed,RestartBackoff,Quarantined };
struct Slot { Service service; Health health; u64 heartbeat; u64 failures; u64 restarts; u64 lastProgress; u64 deadlineEpoch; u64 restartAtEpoch; bool isolated; bool critical; };
struct State { Slot slot[(u32)Service::Count]; u64 epoch; bool ready; };
extern State state;
void Init();void MarkStarting(Service,u64 deadlineTicks=256);void MarkRunning(Service);void Beat(Service);void MarkFailed(Service);void Tick();bool RestartDue(Service);void NoteRestarted(Service);const Slot* Get(Service);
}
