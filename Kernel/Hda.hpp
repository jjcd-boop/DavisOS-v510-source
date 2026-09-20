#pragma once
#include "../Base/Types.hpp"
#include "Pci.hpp"
#include "Dma.hpp"
namespace Davis::Hda {
enum class Result:u32{Cold,NotFound,InvalidBar,Discovered,ResetTimeout,NoCodec,CodecReady,VerbTimeout,VerbError,RingAllocFailed,RingSetupFailed,RingReady,TopologyReady,RouteReady,PlaybackAllocFailed,RouteProgramFailed,PlaybackReady,PlaybackRunning};
enum class WidgetType:u8{AudioOutput=0,AudioInput=1,AudioMixer=2,AudioSelector=3,PinComplex=4,Power=5,VolumeKnob=6,Beep=7,Vendor=15};
struct CodecInfo { u8 address; u32 vendorDevice; u32 revision; u32 rootNodeCount; u8 firstFunctionGroup; u8 functionGroupCount; bool valid; };
static constexpr u32 MaxConnections=16;
struct WidgetInfo {u8 codec,node;WidgetType type;u32 capabilities;u8 connectionCount;u8 connections[MaxConnections];u32 pinCaps;u32 defaultConfig;bool valid;};
struct PlaybackRoute{u8 codec,dac,pin;u8 path[8];u8 pathLength;bool valid;};
struct BdlEntry{u64 address;u32 length;u32 flags;} __attribute__((packed));
static constexpr u32 MaxWidgets=128;
struct State {
 bool discovered,controllerReady,ringsReady; u8 bus,device,function; u16 vendorId,deviceId; u64 mmio; Result result;
 u16 codecMask; CodecInfo codecs[15]; u32 codecCount; u64 resets,verbs,verbTimeouts;
 Memory::DmaBuffer corb,rirb;u16 corbEntries,rirbEntries;u16 corbWrite,rirbRead;u8 rirbPhase;u64 ringVerbs,ringResponses;
 WidgetInfo widgets[MaxWidgets];u32 widgetCount;u32 outputWidgets,pinWidgets,mixerWidgets;
 PlaybackRoute route; Memory::DmaBuffer playbackBdl,playbackData;u8 outputStreamIndex,streamTag;u32 streamOffset,bufferBytes,lastDmaPosition,writeQuarter;u64 playbackStarts,playbackRefills,dmaPositionReads,routePrograms,testToneFrames;bool testToneEnabled,playbackPrepared,playbackRunning;
};
extern State state;
void Init(); bool Discover(const Pci::State& pci); bool ResetController(u32 spins=2000000); bool ProbeCodecs(u32 spins=500000); bool BringUp(const Pci::State& pci);
bool ImmediateVerb(u8 codec,u8 node,u16 verb,u8 payload,u32& response,u32 spins=500000);
bool SetupCommandRings(u32 spins=500000);bool RingCommand(u32 command,u32&response,u32 spins=500000);bool RingVerb(u8 codec,u8 node,u16 verb,u8 payload,u32&response,u32 spins=500000);bool EnumerateTopology(u32 spins=500000);
bool SelectPlaybackRoute(u32 spins=500000);bool ProgramPlaybackRoute(u32 spins=500000);bool PreparePlayback(u32 spins=500000);bool StartPlayback();void StopPlayback();u32 PlaybackPosition();bool SubmitStereo16(const i16* samples,u32 frames);void EnableTestTone(bool enable);void FillTestTone();
}
