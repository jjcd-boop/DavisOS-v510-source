#pragma once
#include "KernelPlatform.hpp"
#include "MultiAppRuntime.hpp"
#include "InterruptAudit.hpp"
namespace Davis::PlatformBootstrap {
enum class Result:u8 { Cold, RuntimeMissing, StackAllocationFailed, GdtFailed, IdtFailed, ApicUnavailable, X2ApicUnsupported, TimeUnavailable, CalibrationFailed, PlatformFailed, Ready };
struct State { Result result; KernelPlatform::State platform; UserMode::GdtState gdt; Idt::Table idt; InterruptController::State controller; LocalApicTimer::State timer; u64 kernelStackPa; u64 kernelRsp0; ApicPlatform::State apic; TimeReference::State time; TimerCalibration::Result calibration; InterruptAudit::State interruptAudit; bool ready; };
extern State state;
bool Prepare(MultiAppRuntime::State&);
}
