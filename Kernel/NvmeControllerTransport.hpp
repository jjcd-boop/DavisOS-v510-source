#pragma once
#include "../Base/Types.hpp"
namespace Davis::NvmeControllerTransport {
constexpr u32 DefaultReadyPollLimit=12000000;
enum class Stage:u8{Cold,Disabled,AdminConfigured,Enabling,Ready,Failed,TimedOut};
struct Capabilities{u16 maxQueueEntries;u8 timeoutUnits500ms;u8 doorbellStrideShift;u32 doorbellStrideBytes;u8 minPageShift,maxPageShift;bool nvmCommandSet;bool valid;};
struct State{volatile u8*mmio;u64 cap;u32 pageBytes;u16 adminDepth;u64 asq,acq;Stage stage;u64 polls;u32 resets;Capabilities capabilities;};
bool DecodeCapabilities(u64,Capabilities&);
bool Bind(State&,void*mmio,u32 pageBytes=4096);
bool Disable(State&,u32 pollLimit);
bool ConfigureAdmin(State&,u64 asq,u64 acq,u16 depth);
bool Enable(State&,u32 pollLimit);
bool ResetAndReconfigure(State&,u64 asq,u64 acq,u16 depth,u32 pollLimit);
}
