#include "BootInfo.hpp"
#include "Graphics.hpp"
#include "Desktop.hpp"
#include "Input.hpp"
#include "KernelMemory.hpp"
#include "AppRuntime.hpp"
#include "MultiAppRuntime.hpp"
#include "DriverSelection.hpp"
#include "PciResources.hpp"
#include "DriverMigration.hpp"
#include "PlatformBootstrap.hpp"
#include "LiveRunner.hpp"
#include "BootAppRunner.hpp"
#include "DesktopService.hpp"
#include "DesktopRuntime.hpp"
#include "BootDiagnostics.hpp"
#include "BackendDiagnostics.hpp"
#include "HardwareProbe.hpp"
#include "SystemTasks.hpp"
#include "StorageService.hpp"
#include "UsbRuntime.hpp"
#include "AudioService.hpp"
#include "NetworkService.hpp"
#include "SystemStatus.hpp"
#include "Syscall.hpp"
#include "I2cHid.hpp"
#include "DiagnosticDump.hpp"
#include "ForgePersistentStore.hpp"
#include "CandidateBoot.hpp"
#include "MediaLibrary.hpp"
#include "KernelSecurity.hpp"
#include "ServiceSupervisor.hpp"
#include "Power.hpp"
#include "../Arch/x86_64/Cpu.hpp"
using namespace Davis;
static inline void BootMark(BootInfo*info,u32 stage,u32 color){
 if(!info||!info->framebuffer.base||!info->framebuffer.width||!info->framebuffer.height)return;
 u32*fb=(u32*)(uptr)info->framebuffer.base;u32 seg=28;u32 x0=stage*seg;u32 x1=x0+24;if(x0>=info->framebuffer.width)return;if(x1>info->framebuffer.width)x1=info->framebuffer.width;u32 h=info->framebuffer.height<12?info->framebuffer.height:12;
 for(u32 y=0;y<h;y++)for(u32 x=x0;x<x1;x++)fb[(usize)y*info->framebuffer.pixelsPerScanLine+x]=color;
}
extern "C" __attribute__((ms_abi,section(".text.entry"))) [[noreturn]] void DavisKernelMain(BootInfo*info){
 Cpu::DisableInterrupts();
 KernelSecurity::Init();
 ServiceSupervisor::Init();
 DriverMigration::Init();
 BackendDiagnostics::Init();BackendDiagnostics::Record(BackendDiagnostics::Kind::Boot,0,1,0);
 if(info){Power::Init(*info);CandidateBoot::Mark(*info,CandidateBoot::Checkpoint::KernelEntry);HardwareProbe::Begin(*info);if(info->hardwareProbeRequested){HardwareProbe::ScanAndDisplay(*info);HardwareProbe::CompleteAndHalt();}BootDiagnostics::Begin(*info);BootDiagnostics::Pass("Kernel entry");}
 if(info){KernelMemory::Init(*info);BootMark(info,2,0x0000FF00);MediaLibrary::Init(*info);BootMark(info,3,0x0000FF00);CandidateBoot::Mark(*info,CandidateBoot::Checkpoint::MemoryReady);}
 // v1.54 INPUT LOCKDOWN: restore the exact pre-desktop platform/input ordering from
 // v1.49, the last hardware-tested build where keyboard + native touchpad worked.
 // This sequence is now an invariant. Storage/audio/network remain deferred.
 if(info){MultiAppRuntime::Prepare(*info);BootDiagnostics::Pass("DXE runtime structures prepared");CandidateBoot::Mark(*info,CandidateBoot::Checkpoint::RuntimeReady);}
 if(info){PlatformBootstrap::Prepare(MultiAppRuntime::state);BootDiagnostics::Pass("CPU platform tables prepared");} BootMark(info,5,0x0000FF00);
 Syscall::InitObjectHandles();
 DriverSelection::Init(); DriverSelection::Discover(); PciResources::PublishAll(); SystemStatus::Init(); BootDiagnostics::Pass("PCI driver candidates/resources published"); if(info)CandidateBoot::Mark(*info,CandidateBoot::Checkpoint::HardwareReady);
 DesktopService::Init();
 // INPUT TRANSPORT ORDER INVARIANT: xHCI first, I2C-HID second, PS/2 last inside
 // DesktopRuntime::Init/Input::Init. Do not reorder without a hardware regression test.
 UsbRuntime::Init();BootDiagnostics::Pass("USB/xHCI input transport returned");
 if(info){I2cHid::Init(*info);BootDiagnostics::Note(I2cHid::StageName(I2cHid::state.stage));CandidateBoot::Mark(*info,CandidateBoot::Checkpoint::InputReady);}
 // v1.55 isolation recovery: background services are not dispatched from the desktop.
 // SystemTasks remains available for the future preemptive/supervised path only.
 SystemTasks::Init(4);
 SystemTasks::Configure(SystemTasks::Kind::Storage,8,80,1,false);
 NetworkService::Init();
 SystemTasks::Configure(SystemTasks::Kind::Network,4,70,1,NetworkService::state.nicReady);
 SystemTasks::Configure(SystemTasks::Kind::Audio,2,90,1,false);
 if(info){DesktopRuntime::Init(*info);ServiceSupervisor::MarkRunning(ServiceSupervisor::Service::Desktop);ServiceSupervisor::MarkRunning(ServiceSupervisor::Service::Input);BootDiagnostics::Pass("Desktop runtime initialized");BootMark(info,8,0x0000FF00);DiagnosticDump::Init(*info);ForgePersistentStore::Init(*info);DiagnosticDump::Snapshot("POST_DESKTOP_INIT");CandidateBoot::Mark(*info,CandidateBoot::Checkpoint::DesktopReady);}
 if(info&&info->diagnosticBootRequested)BootDiagnostics::CompleteAndHalt();
 if(info&&info->liveProofRequested)LiveRunner::TryStart(*info,MultiAppRuntime::state,PlatformBootstrap::state);
 // v1.61: production Ring-3 driver/app cohort. Timer preemption prevents any one
 // user process from monopolizing the CPU; cooperative yields return through the
 // protected trap bridge so the kernel can compose/poll input between slices.
 if(info){BackendDiagnostics::Record(BackendDiagnostics::Kind::Boot,0,0x4C415A59,MultiAppRuntime::state.appCount,MultiAppRuntime::state.bootManifestCount);CandidateBoot::Mark(*info,CandidateBoot::Checkpoint::Ring3Ready);CandidateBoot::AcknowledgeHealthy(*info);}
 if(DesktopRuntime::state.ready)DesktopRuntime::Run();Cpu::HaltForever();
}
extern "C" void* memset(void* p,int v,unsigned long n){auto*d=(unsigned char*)p;while(n--)*d++=(unsigned char)v;return p;}
extern "C" void* memcpy(void* d,const void* s,unsigned long n){auto*a=(unsigned char*)d;auto*b=(const unsigned char*)s;while(n--)*a++=*b++;return d;}
