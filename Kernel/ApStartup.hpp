#pragma once
#include "../Base/Types.hpp"
namespace Davis::ApStartup {
constexpr u32 MaxSequenceSteps=4;
enum class Kind:u8 { InitAssert, InitDeassert, Startup, StartupRetry };
struct Step { Kind kind; u32 apicId; u8 vector; u32 delayUs; u64 icr; };
struct Plan { Step steps[MaxSequenceSteps]; u32 count; u64 trampolinePhysical; bool valid; };
bool BuildPlan(u32 apicId,u64 trampolinePhysical,Plan&);
bool ValidateTrampoline(u64 physical);
u64 EncodeX2ApicIcr(Kind,u32 apicId,u8 vector);
u64 EncodeXApicIcr(Kind,u32 apicId,u8 vector);
}
