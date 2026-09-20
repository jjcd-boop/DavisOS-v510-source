#pragma once
#include "BootInfo.hpp"
#include "DxeLoader.hpp"
#include "Preemption.hpp"
#include "PreemptionProof.hpp"
namespace Davis::MultiAppRuntime {
enum class Result:u32 { Disabled, MemoryUnavailable, MissingProofApps, PrepareFailed, KernelContractFailed, InterruptContractFailed, ContextFailed, Armed };
enum class AppStage:u32 { BootImage=1,DxePrepare=2,KernelStack=3,KernelContract=4,InterruptContract=5,UserContext=6,Ready=7 };
struct App { u32 kind; DxeLoader::Loaded loaded; CpuContext::Frame initial; u64 kernelStackGuardPa; u64 kernelStackPa; u64 kernelRsp0; volatile u64* kernelStackCanary; bool ready; AppStage stage; u32 error; };
static constexpr usize MaxRuntimeApps=64;
struct BootManifestEntry { u32 kind; u64 base; u64 bytes; };
struct State { BootManifestEntry bootManifest[MaxBootApps]; usize bootManifestCount; u64 bootManifestSeal; Process::Table processes; Scheduler::State scheduler; Preemption::State preemption; PreemptionProof::State proof; App apps[MaxRuntimeApps]; usize appCount; Result result; bool armed; };
extern State state;
void Prepare(const BootInfo&);
bool BootManifestIntact();
u64 PidForKind(u32 kind);
Process::Image* ProcessForKind(u32 kind);
const App* AppForKind(u32 kind);
void BeginShutdown();
// Clone a boot-loaded template into a fresh address space and scheduler context.
u64 SpawnIsolatedFromKind(u32 kind);
}
