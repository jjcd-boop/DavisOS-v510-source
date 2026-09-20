#pragma once
#include "../Base/Types.hpp"
namespace Davis::DriverMigration {
enum class Driver:u32{Audio,Network,Storage,Usb,Input,Graphics,Wifi,Count};
enum class Mode:u32{KernelCompatibility=0,Ring3Prepared=1,Ring3Active=2};
struct Entry{Driver driver;Mode mode;u64 processId;bool bootImagePresent;};
struct State{Entry entries[(u32)Driver::Count];bool ready;};
extern State state;void Init();void NotePrepared(Driver,u64);void NoteActive(Driver,u64);
}
