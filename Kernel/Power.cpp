#include "Power.hpp"
#include "../Arch/x86_64/Cpu.hpp"
#include "MultiAppRuntime.hpp"
#include "BlockDevice.hpp"
namespace Davis::Power {
using ResetFn=void(__attribute__((ms_abi)) *)(u32,u64,usize,const void*);
struct RtPrefix { u8 hdr[24]; void*getTime;void*setTime;void*getWakeupTime;void*setWakeupTime;void*setVirtualAddressMap;void*convertPointer;void*getVariable;void*getNextVariableName;void*setVariable;void*getNextHighMonotonicCount;ResetFn resetSystem; };
static RtPrefix*rt=nullptr;
void Init(const BootInfo&b){rt=(RtPrefix*)b.runtimeServices;}
[[noreturn]] void Shutdown(){
 // v420 orderly shutdown barrier. Stop Ring-3 ownership first, then drain all
 // block requests before firmware power-off. Physical media is still read-only.
 MultiAppRuntime::BeginShutdown();
 for(u32 spin=0;spin<4096;spin++)BlockDevice::PumpOnce();
 for(u32 i=0;i<BlockDevice::MaxDevices;i++)if(BlockDevice::state.devices[i].present)BlockDevice::CancelDeviceRequests(BlockDevice::state.devices[i].id,-31);
 Cpu::DisableInterrupts();
 if(rt&&rt->resetSystem)rt->resetSystem(2,0,0,nullptr);
 Cpu::HaltForever();
}
}
