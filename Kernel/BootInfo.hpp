#pragma once
#include "../Base/Types.hpp"
namespace Davis {
struct Framebuffer{uptr base;u32 width,height,pixelsPerScanLine;u32 pixelFormat;};
struct BootApp { uptr base; usize bytes; u32 kind; u32 reserved; };
static constexpr usize MaxBootApps=32;
struct BootMedia { uptr base; usize bytes; u32 kind; char path[128]; };
static constexpr usize MaxBootMedia=24;
enum BootMediaKind:u32 { MediaOther=0, MediaAudio=1, MediaImage=2 };
enum BootAppKind:u32 { AppGeneric=0, AppHello=1, AppPreemptionA=2, AppPreemptionB=3, AppDriverAudio=16, AppDriverNetwork=17, AppDriverStorage=18, AppDriverUsb=19, AppDriverInput=20, AppDriverGraphics=21, AppDriverWifi=22, AppWriter=32, AppCalculator=33, AppFiles=34, AppTerminal=35, AppSettings=36, AppBrowser=37, AppMedia=38, AppPhotos=39, AppSkyDefender=40, AppSolitaire=41, AppChess=42, AppCheckers=43, AppWifi=44, AppCauseway=45, AppPebble=46, AppMorrow=47, AppForge=48, AppForgeRunner=49 };
struct BootInfo{
 Framebuffer framebuffer;uptr rsdp;uptr memoryMap;usize memoryMapBytes,descriptorSize;u32 descriptorVersion;
 uptr appBase;usize appBytes;bool appLaunchDisabled; // v0.64 compatibility alias for HelloDavis
 bool liveProofRequested; // v0.89 explicit opt-in: DAVIS/ENABLE_LIVE_PROOF.TXT
 bool diagnosticBootRequested; // DAVIS/DIAGNOSTIC_BOOT.TXT
 bool safeBootRequested; // v1.51: DAVIS/SAFE_BOOT.TXT
 bool hardwareProbeRequested; // v1.51: DAVIS/HARDWARE_PROBE.TXT
 bool physicalStorageReadOnly; // v1.51: default true; ALLOW_PHYSICAL_WRITES.TXT explicitly opts in later
 bool extensiveDumpRequested; // v1.42: persistent diagnostic journal via UEFI variable
 bool candidateBoot; // v23: booted disposable Forge candidate slot
 u32 candidateStageDigest; // v23: authenticated candidate transaction identity
 uptr runtimeServices; // UEFI Runtime Services pointer, identity-mapped after ExitBootServices
 u8 reserved0[7]; BootApp apps[MaxBootApps]; usize appCount; BootMedia media[MaxBootMedia]; usize mediaCount;
};
}
