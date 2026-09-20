#pragma once
#include "../Base/Types.hpp"
#include "Nvme.hpp"
namespace Davis::NvmeAdminRuntime {
constexpr u32 DefaultPollLimit=1000000;
enum class Stage:u8 { Cold, CqSubmitted, CqReady, SqSubmitted, Ready, Failed, TimedOut };
struct Transport { bool (*submit)(void*,const Nvme::IoCommand&); bool (*poll)(void*,u16,u16&); void (*ring)(void*,bool,u16); void (*reset)(void*); void* context; };
struct State { Stage stage; u16 cqCid,sqCid; u32 polls; u32 resets; bool ready; };
bool Validate(const Transport&); bool CreateIoQueues(const Transport&,const Nvme::QueueCreatePlan&,u16 cqCid,u16 sqCid,u32 pollLimit,State&);
}
