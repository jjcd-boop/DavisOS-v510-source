#pragma once
#include "../Base/Types.hpp"
#include "Process.hpp"
#include "ObjectHandles.hpp"
namespace Davis::Syscall {
enum Number:u64 { Exit=0,Log=1,Yield=2,GetPid=3,CreateWindow=16,PresentWindow=17,OpenFile=32,ReadFile=33,WriteFile=34,CloseFile=35,ListDirectory=36,ServiceLookup=48,IpcCreate=49,IpcSend=50,IpcReceive=51,IpcClose=52,ServiceRegister=53,SpawnIsolated=54,ForgeStoreRead=55,ForgeStoreWrite=56,ForgeRunReport=57,ForgeRunQuery=58,ForgePackageInstall=59,ForgePackageRead=60,ForgePackageRemove=61,ForgePackageCount=62,ForgePackageFetch=63,DriverRegister=64,DriverClaimResource=65,DriverReleaseResource=66,DriverHeartbeat=67,DriverMapMmio=68,DriverPortRead=69,DriverPortWrite=70,DriverBindIrq=71,DriverIdentity=72,DriverResourceInfo=73,DriverAllocDma=74,DriverFreeDma=75,DriverConfigureMsi=76,DriverTakeOwnership=77,AllocMemory=80,FreeMemory=81 };
struct Frame { u64 number,arg0,arg1,arg2,arg3,arg4,arg5; };
struct Result { i64 value;u64 error; };
static constexpr u64 ErrorPermissionDenied=13;
static constexpr u64 ErrorBadHandle=9;
static constexpr u64 ErrorNoResources=12;
static constexpr u64 ErrorNotSupported=95;
void InitObjectHandles();
ObjectHandles::Table& HandlesForTest();
void RevokeProcessObjects(u64 owner);
Result Dispatch(const Frame&);
Result DispatchForProcess(const Frame&,const Process::Image*);
u64 RequiredCapability(u64 syscallNumber);
}
