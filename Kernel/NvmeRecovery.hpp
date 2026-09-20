#pragma once
#include "../Base/Types.hpp"
namespace Davis::NvmeRecovery {
constexpr u32 DefaultMaxResetAttempts=3;
enum class Stage:u8 { Healthy,Quiescing,Resetting,Reinitializing,Recovered,Failed };
struct Hooks { bool(*quiesce)(void*); bool(*reset)(void*); bool(*reinitialize)(void*); void* context; };
struct State { Stage stage; u32 attempts,maxAttempts; u64 recoveries,failures; };
bool Init(State&,u32 maxAttempts=DefaultMaxResetAttempts); bool Recover(State&,const Hooks&);
}
