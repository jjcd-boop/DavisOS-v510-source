#include "MultiAppRuntime.hpp"
#include "StackGuard.hpp"
#include "StackRuntime.hpp"
#include "KernelMemory.hpp"
#include "UserLaunch.hpp"
#include "AddressSpace.hpp"
#include "ApicPlatform.hpp"
#include "InterruptAddressSpace.hpp"
#include "UserMode.hpp"
#include "ExceptionRecovery.hpp"
#include "DriverMigration.hpp"
#include "DriverManager.hpp"
#include "Pci.hpp"
#include "Syscall.hpp"
#include "BackendDiagnostics.hpp"
namespace Davis::MultiAppRuntime {
State state{};
static UserMode::GdtState modelGdt{};
static App* byKind(u32 kind){for(usize i=0;i<state.appCount;i++)if(state.apps[i].kind==kind)return &state.apps[i];return nullptr;}
static bool driverKind(u32 k){return k>=AppDriverAudio&&k<=AppDriverWifi;}
u64 PidForKind(u32 kind){auto*a=byKind(kind);return a&&a->loaded.process?a->loaded.process->id:0;}
Process::Image* ProcessForKind(u32 kind){auto*a=byKind(kind);return a?a->loaded.process:nullptr;}
const App* AppForKind(u32 kind){return byKind(kind);}
void BeginShutdown(){for(usize i=0;i<state.appCount;i++){auto*p=state.apps[i].loaded.process;if(!p)continue;if(p->state==Process::State::Ready||p->state==Process::State::Running||p->state==Process::State::Blocked||p->state==Process::State::Created){Syscall::RevokeProcessObjects(p->id);Process::MarkExited(*p,0);}}state.armed=false;}
u64 SpawnIsolatedFromKind(u32 kind){
 if(state.appCount>=MaxRuntimeApps||!KernelMemory::ready)return 0;
 App*tmpl=byKind(kind);if(!tmpl||!tmpl->loaded.process||!tmpl->loaded.process->fileImage||!tmpl->loaded.process->fileBytes)return 0;
 auto&a=state.apps[state.appCount];a={};a.kind=kind;
 if(DxeLoader::PrepareDxe2AsDomain(state.processes,KernelMemory::allocator,tmpl->loaded.process->fileImage,tmpl->loaded.process->fileBytes,a.loaded,SecurityDomain::Kind::SystemService)!=DxeLoader::Result::Ok)return 0;
 constexpr u64 kp=8;StackGuard::Stack gs{};if(!StackGuard::Allocate(gs,KernelMemory::allocator,kp))return 0;
 a.kernelStackGuardPa=gs.guardBase;a.kernelStackPa=gs.usableBase;a.kernelRsp0=gs.top;a.kernelStackCanary=gs.canary;
 if(!StackRuntime::Register(a.loaded.process->id,gs))return 0;
 if(AddressSpace::InstallKernelContract(a.loaded.space,KernelMemory::allocator,a.kernelStackPa,kp)!=AddressSpace::Result::Ok||!AddressSpace::VerifyKernelContract(a.loaded.space,a.kernelStackPa,kp)||!StackGuard::MapInto(gs,a.loaded.space,KernelMemory::allocator))return 0;
 a.loaded.sharedKernelMappings=true;auto apic=ApicPlatform::Probe();auto irq=InterruptAddressSpace::FromApic(apic);if(apic.usable&&(!InterruptAddressSpace::Install(irq,a.loaded.space,KernelMemory::allocator)||!InterruptAddressSpace::Verify(irq,a.loaded.space)))return 0;
 UserMode::BuildGdt(modelGdt,a.kernelRsp0);UserLaunch::Frame uf{};if(!UserLaunch::Build(uf,a.loaded.process->entryVirtual,a.loaded.stackTop,a.loaded.space.pml4Physical,modelGdt))return 0;
 a.initial={};a.initial.rip=uf.rip;a.initial.rsp=uf.rsp;a.initial.rflags=uf.rflags;a.initial.cs=uf.cs;a.initial.ss=uf.ss;
 if(!Preemption::SaveExecution(state.preemption,a.loaded.process->id,a.initial,a.loaded.space.pml4Physical,a.kernelRsp0)||!Preemption::BindAddressSpace(state.preemption,a.loaded.process->id,a.loaded.space.pml4Physical,a.kernelRsp0,true))return 0;
 a.ready=true;state.appCount++;return a.loaded.process->id;
}
static u64 manifestSeal(const State&s){u64 h=0xD4515A11C0DEC0DEull;for(usize i=0;i<s.bootManifestCount;i++){const auto&e=s.bootManifest[i];h^=((u64)e.kind<<48)^e.base^(e.bytes<<1);h=(h<<13)|(h>>(64-13));h*=0x9E3779B185EBCA87ull;}return h;}
bool BootManifestIntact(){return state.bootManifestSeal==manifestSeal(state);}
void Prepare(const BootInfo&b){state={};for(usize i=0;i<b.appCount&&i<MaxBootApps;i++){const auto&src=b.apps[i];if(!src.base||!src.bytes)continue;auto&e=state.bootManifest[state.bootManifestCount++];e.kind=src.kind;e.base=src.base;e.bytes=src.bytes;}state.bootManifestSeal=manifestSeal(state);BackendDiagnostics::Record(BackendDiagnostics::Kind::Boot,0,0xB007,state.bootManifestCount,state.bootManifestSeal);for(usize mi=0;mi<state.bootManifestCount;mi++){const auto&m=state.bootManifest[mi];BackendDiagnostics::Record(BackendDiagnostics::Kind::Boot,0,0x49444D50,m.kind,m.base,m.bytes);}if(b.appLaunchDisabled){state.result=Result::Disabled;return;}if(!KernelMemory::ready){state.result=Result::MemoryUnavailable;return;}Process::Init(state.processes);Scheduler::Init(state.scheduler,state.processes,{1});Preemption::Init(state.preemption,state.scheduler);
 auto apic=ApicPlatform::Probe();auto irq=InterruptAddressSpace::FromApic(apic);
 for(usize i=0;i<b.appCount&&i<MaxBootApps;i++){auto&src=b.apps[i];if(!src.base||!src.bytes)continue;auto&a=state.apps[state.appCount++];a={};a.kind=src.kind;a.stage=AppStage::BootImage;if((driverKind(src.kind)?DxeLoader::PrepareDxe2AsDomain(state.processes,KernelMemory::allocator,(void*)src.base,src.bytes,a.loaded,SecurityDomain::Kind::Driver):(src.kind==AppForgeRunner?DxeLoader::PrepareDxe2AsDomain(state.processes,KernelMemory::allocator,(void*)src.base,src.bytes,a.loaded,SecurityDomain::Kind::SystemService):DxeLoader::Prepare(state.processes,KernelMemory::allocator,(void*)src.base,src.bytes,a.loaded)))!=DxeLoader::Result::Ok){a.stage=AppStage::DxePrepare;a.error=1;continue;}a.stage=AppStage::DxePrepare;constexpr u64 kp=8;StackGuard::Stack gs{};if(!StackGuard::Allocate(gs,KernelMemory::allocator,kp)){a.stage=AppStage::KernelStack;a.error=1;continue;}a.stage=AppStage::KernelStack;a.kernelStackGuardPa=gs.guardBase;a.kernelStackPa=gs.usableBase;a.kernelRsp0=gs.top;a.kernelStackCanary=gs.canary;if(!StackRuntime::Register(a.loaded.process->id,gs)){a.stage=AppStage::KernelContract;a.error=1;continue;}if(AddressSpace::InstallKernelContract(a.loaded.space,KernelMemory::allocator,a.kernelStackPa,kp)!=AddressSpace::Result::Ok||!AddressSpace::VerifyKernelContract(a.loaded.space,a.kernelStackPa,kp)||!StackGuard::MapInto(gs,a.loaded.space,KernelMemory::allocator)){a.stage=AppStage::KernelContract;a.error=2;continue;}a.stage=AppStage::KernelContract;a.loaded.sharedKernelMappings=true;if(apic.usable&&(!InterruptAddressSpace::Install(irq,a.loaded.space,KernelMemory::allocator)||!InterruptAddressSpace::Verify(irq,a.loaded.space))){a.stage=AppStage::InterruptContract;a.error=1;continue;}a.stage=AppStage::InterruptContract;
  UserMode::BuildGdt(modelGdt,a.kernelRsp0);UserLaunch::Frame uf{};if(!UserLaunch::Build(uf,a.loaded.process->entryVirtual,a.loaded.stackTop,a.loaded.space.pml4Physical,modelGdt)){a.stage=AppStage::UserContext;a.error=1;continue;}a.stage=AppStage::UserContext;a.initial={};a.initial.rip=uf.rip;a.initial.rsp=uf.rsp;a.initial.rflags=uf.rflags;a.initial.cs=uf.cs;a.initial.ss=uf.ss;if(!Preemption::SaveExecution(state.preemption,a.loaded.process->id,a.initial,a.loaded.space.pml4Physical,a.kernelRsp0)||!Preemption::BindAddressSpace(state.preemption,a.loaded.process->id,a.loaded.space.pml4Physical,a.kernelRsp0,true)){a.stage=AppStage::UserContext;a.error=2;continue;}a.ready=true;a.stage=AppStage::Ready;BackendDiagnostics::Record(BackendDiagnostics::Kind::AppPrepare,a.loaded.process?a.loaded.process->id:0,a.kind,(u64)a.stage);if(driverKind(a.kind)&&a.loaded.process){auto d=(DriverMigration::Driver)(a.kind-AppDriverAudio);u64 dev=0xA001ull+(u64)(a.kind-AppDriverAudio); Pci::State pci{}; Pci::Scan(pci); const Pci::Device* hw=nullptr; if(a.kind==AppDriverAudio&&pci.hdaFound)hw=&pci.hda; else if(a.kind==AppDriverNetwork&&pci.ethernetFound)hw=&pci.ethernet; else if(a.kind==AppDriverStorage)hw=pci.nvmeFound?&pci.nvme:(pci.ahciFound?&pci.ahci:nullptr); else if(a.kind==AppDriverUsb&&pci.xhciFound)hw=&pci.xhci; else if(a.kind==AppDriverGraphics&&pci.displayFound)hw=&pci.display; else if(a.kind==AppDriverWifi){for(u32 pi=0;pi<pci.count;pi++)if(pci.devices[pi].classCode==0x02&&pci.devices[pi].subclass==0x80){hw=&pci.devices[pi];break;}} if(hw)dev=0xD000000000000000ull|((u64)hw->vendor<<32)|((u64)hw->device<<16)|((u64)hw->bus<<8)|((u64)hw->slot<<3)|hw->function; if(!DriverManager::AssignTrusted(a.loaded.process->id,dev)){a.ready=false;a.error=0xD001;BackendDiagnostics::Record(BackendDiagnostics::Kind::Error,a.loaded.process->id,a.kind,0xD001,dev);continue;}DriverMigration::NotePrepared(d,a.loaded.process->id);}}
 auto*A=byKind(AppPreemptionA);auto*B=byKind(AppPreemptionB);if(A&&B){PreemptionProof::Endpoint ea{A->loaded.process->id,A->loaded.space.pml4Physical,A->kernelRsp0,A->loaded.sharedKernelMappings};PreemptionProof::Endpoint eb{B->loaded.process->id,B->loaded.space.pml4Physical,B->kernelRsp0,B->loaded.sharedKernelMappings};if(!PreemptionProof::Arm(state.proof,ea,eb)){state.result=Result::ContextFailed;return;}state.armed=true;}ExceptionRecovery::Configure(state.scheduler,state.preemption);Preemption::SetGlobal(&state.preemption);state.result=Result::Armed;
}
}
