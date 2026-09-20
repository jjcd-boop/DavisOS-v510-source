#include "PlatformBootstrap.hpp"
#include "PciInterrupts.hpp"
#include "KernelMemory.hpp"
#include "Exceptions.hpp"
#include "UserTrap.hpp"
#include "LapicCalibration.hpp"
#include "EmergencyIst.hpp"
namespace Davis::PlatformBootstrap {
State state{};
bool Prepare(MultiAppRuntime::State&r){
 state={};
 if(!r.armed||r.result!=MultiAppRuntime::Result::Armed){state.result=Result::RuntimeMissing;return false;}
 constexpr u64 stackPages=16; state.kernelStackPa=Memory::AllocPages(KernelMemory::allocator,stackPages,1);
 if(!state.kernelStackPa){state.result=Result::StackAllocationFailed;return false;} state.kernelRsp0=state.kernelStackPa+stackPages*4096;
 UserMode::BuildGdt(state.gdt,state.kernelRsp0); if(!UserMode::Validate(state.gdt)){state.result=Result::GdtFailed;return false;}
 Idt::Init(state.idt); Exceptions::InstallCoreGates(state.idt,state.gdt.selectors.kernelCode); if(!EmergencyIst::Prepare(state.gdt,state.idt,state.gdt.selectors.kernelCode)){state.result=Result::IdtFailed;return false;} for(usize i=0;i<r.appCount;i++){if(r.apps[i].ready&&!EmergencyIst::MapInto(r.apps[i].loaded.space,KernelMemory::allocator)){state.result=Result::IdtFailed;return false;}} Idt::SetInterruptGate(state.idt,0x80,UserTrap::DavisInt80Entry,state.gdt.selectors.kernelCode,3); PciInterrupts::InstallGates(state.idt,state.gdt.selectors.kernelCode);
 if(!Idt::ValidateSyscallGate(state.idt,0x80,state.gdt.selectors.kernelCode)){state.result=Result::IdtFailed;return false;}
 InterruptController::InitNone(state.controller); LocalApicTimer::Init(state.timer); KernelPlatform::Init(state.platform,250,LocalApicTimer::DefaultVector);
 if(!KernelPlatform::BindCore(state.platform,state.gdt,state.idt,r.scheduler,r.preemption,state.controller,state.timer)){state.result=Result::PlatformFailed;return false;}
 state.apic=ApicPlatform::Probe(); if(!state.apic.usable){state.result=Result::ApicUnavailable;return false;}
 if(!KernelPlatform::AcceptApic(state.platform,state.apic)){state.result=Result::PlatformFailed;return false;}
 state.time=TimeReference::Probe(); if(!state.time.usable||!KernelPlatform::AcceptTime(state.platform,state.time)){state.result=Result::TimeUnavailable;return false;}
 auto measured=state.apic.x2Apic?LapicCalibration::MeasureX2(state.time,10000,250,0x3):LapicCalibration::Measure(state.controller.lapic,state.time,10000,250,0x3); if(!measured.valid){state.result=Result::CalibrationFailed;return false;} state.calibration=measured.timer;
 if(!KernelPlatform::AcceptCalibration(state.platform,state.calibration)){state.result=Result::CalibrationFailed;return false;}
 TimerRuntime::Init(state.platform.runtime); if(!KernelPlatform::ValidateReady(state.platform)){state.result=Result::PlatformFailed;return false;}
 if(!InterruptAudit::Verify(state.interruptAudit,state.idt,state.gdt.selectors.kernelCode,state.apic,state.controller,state.timer,state.platform.timerPlatform)){state.result=Result::PlatformFailed;return false;}
 // Load owned CPU tables only after every structure and gate validates. Interrupts remain disabled.
 UserMode::LoadGdtAndTss(state.gdt); Idt::Load(state.idt); InterruptController::SetGlobal(&state.controller);
 KernelPlatform::PermitLiveActivation(state.platform,false); state.ready=true;state.result=Result::Ready;return true;
}
}
