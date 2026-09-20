#include "AppRuntime.hpp"
#include "KernelMemory.hpp"
#include "DxeLoader.hpp"
#include "UserMode.hpp"
#include "UserLaunch.hpp"
#include "Idt.hpp"
#include "Exceptions.hpp"
#include "UserTrap.hpp"
#include "TrapExit.hpp"
#include "TrapReturn.hpp"
#include "PlatformState.hpp"
#include "AddressSpace.hpp"
#include "ApicPlatform.hpp"
#include "InterruptAddressSpace.hpp"
namespace Davis::AppRuntime {
Status status{};
static Process::Table processes{};
static UserMode::GdtState gdt{};
static Idt::Table idt{};
extern "C" void DavisInt80Entry();
extern "C" void DavisEnterPreparedUser(u64,u64,u64,u64,u64,u64);
static bool mapKernel(DxeLoader::Loaded&l,u64 stackPa,u64 stackPages){
 return AddressSpace::InstallKernelContract(l.space,KernelMemory::allocator,stackPa,stackPages)==AddressSpace::Result::Ok;
}

void TryHelloDavis(const BootInfo&b){status={};if(b.appLaunchDisabled){status.result=Result::Disabled;return;}if(!b.appBase||!b.appBytes){status.result=Result::NotPresent;return;}status.attempted=true;if(!KernelMemory::ready){status.result=Result::MemoryUnavailable;return;}
 Process::Init(processes);DxeLoader::Loaded l{};if(DxeLoader::Prepare(processes,KernelMemory::allocator,(void*)b.appBase,b.appBytes,l)!=DxeLoader::Result::Ok){status.result=Result::PrepareFailed;return;}
 constexpr u64 ksPages=8;u64 ks=Memory::AllocPages(KernelMemory::allocator,ksPages,1);if(!ks){status.result=Result::MemoryUnavailable;return;}if(!mapKernel(l,ks,ksPages)){status.result=Result::MapKernelFailed;return;}l.sharedKernelMappings=AddressSpace::VerifyKernelContract(l.space,ks,ksPages);if(!l.sharedKernelMappings){status.result=Result::MapKernelFailed;return;}
 // A timer IRQ can arrive under the process CR3. Preserve the LAPIC MMIO page
 // as supervisor-only/NX before any future live preemption path is permitted.
 auto apic=ApicPlatform::Probe();auto irqContract=InterruptAddressSpace::FromApic(apic);if(apic.usable&&!InterruptAddressSpace::Install(irqContract,l.space,KernelMemory::allocator)){status.result=Result::MapKernelFailed;return;}if(apic.usable&&!InterruptAddressSpace::Verify(irqContract,l.space)){status.result=Result::MapKernelFailed;return;}
 UserMode::BuildGdt(gdt,ks+ksPages*4096);if(!UserMode::Validate(gdt)){status.result=Result::PlatformFailed;return;}Idt::Init(idt);Idt::SetInterruptGate(idt,0x80,DavisInt80Entry,gdt.selectors.kernelCode,3);Exceptions::InstallCoreGates(idt,gdt.selectors.kernelCode);if(!Idt::ValidateSyscallGate(idt,0x80,gdt.selectors.kernelCode)){status.result=Result::PlatformFailed;return;}
 UserMode::LoadGdtAndTss(gdt);auto platform=PlatformState::Capture(gdt);if(!PlatformState::ValidateOwned(platform,gdt)){status.result=Result::PlatformFailed;return;}Idt::Load(idt);UserTrap::SetCurrent(l.process);Exceptions::SetCurrent(l.process);TrapExit::Arm(l.process);UserLaunch::Frame f{};if(!UserLaunch::Build(f,l.process->entryVirtual,l.stackTop,l.space.pml4Physical,gdt)){status.result=Result::PlatformFailed;return;}
 l.process->state=Process::State::Running;status.enteredUser=true;u64 oldCr3=UserLaunch::ReadCr3();DavisEnterPreparedUser(f.rip,f.rsp,f.rflags,f.cs,f.ss,f.cr3); // returns only through kernel continuation
 UserLaunch::WriteCr3(oldCr3);status.result=Result::Returned;status.returnedKernel=true;status.syscallCount=UserTrap::state.syscallCount;status.exitCode=l.process->exitCode;status.exited=l.process->state==Process::State::Exited;status.faulted=l.process->state==Process::State::Faulted;status.faultVector=Exceptions::state.lastVector;TrapExit::Clear();
}
}
