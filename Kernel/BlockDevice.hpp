#pragma once
#include "../Base/Types.hpp"
namespace Davis::BlockDevice {
static constexpr u32 SectorBytes=512;
static constexpr u32 MaxDevices=8;
static constexpr u32 MaxRequests=32;
static constexpr u32 RamDiskSectors=256;
enum class Op:u8 { Read=0, Write=1, Flush=2 };
enum class Status:u8 { Free=0, Queued=1, Active=2, Complete=3, Failed=4, TimedOut=5 };
struct Device { u32 id; u64 sectorCount; u32 sectorBytes; bool writable; bool present; u32 backendId; };
struct Request { u64 id; u32 deviceId; Op op; Status status; u64 lba; u32 sectorCount; u8* buffer; u64 submittedTick; u64 deadlineTick; i32 error; u64 backendToken; };
using SubmitBackend=bool(*)(const Device&,Request&);
using PollBackend=bool(*)(const Device&,Request&,bool& done,bool& success,i32& error);
using CancelBackend=void(*)(const Device&,Request&);
struct Backend { u32 id; SubmitBackend submit; PollBackend poll; CancelBackend cancel; bool present; };
struct State { Device devices[MaxDevices]; Request requests[MaxRequests]; Backend backends[MaxDevices]; u64 nextRequestId; u64 completed; u64 failed; u64 timedOut; u64 ticks; bool ready; };
extern State state;
void Init();
const Device* GetDevice(u32 id);
bool RegisterBackend(u32 backendId,SubmitBackend,PollBackend,CancelBackend=nullptr);
bool RegisterDevice(u32 id,u64 sectors,u32 sectorBytes,bool writable,u32 backendId);
bool RemoveDevice(u32 id,bool cancelOutstanding=true);
bool RemoveBackend(u32 backendId);
u32 CancelDeviceRequests(u32 deviceId,i32 error=-9);
u64 Submit(u32 deviceId,Op op,u64 lba,u32 sectorCount,u8* buffer,u64 timeoutTicks=128);
Status Query(u64 requestId,i32* error=nullptr);
bool Reap(u64 requestId,i32* error=nullptr);
void PumpOnce();
}
