
#include "AudioService.hpp"
#include "AudioCore.hpp"
#include "Hda.hpp"
#include "Pci.hpp"
#include "AudioStream.hpp"
namespace Davis::AudioService {
State state{};
static void speakerGate(bool on){u8 v=0;__asm__ __volatile__("inb $0x61,%0":"=a"(v));if(on)v|=3u;else v=(u8)(v&~3u);__asm__ __volatile__("outb %0,$0x61"::"a"(v));}
static void speakerTone(u32 hz){if(!hz){speakerGate(false);return;}u32 div=1193182u/hz;if(!div)div=1;__asm__ __volatile__("outb %0,$0x43"::"a"((u8)0xB6));__asm__ __volatile__("outb %0,$0x42"::"a"((u8)(div&255u)));__asm__ __volatile__("outb %0,$0x42"::"a"((u8)((div>>8)&255u)));speakerGate(true);}
void Init(){state={};Audio::Init();Pci::State p{};Pci::Scan(p);state.hdaPresent=p.hdaFound;state.hdaReady=Hda::BringUp(p);state.codecs=Hda::state.codecCount;if(state.hdaReady&&Hda::state.playbackPrepared)Hda::EnableTestTone(false);state.ready=true;}void PumpOnce(){if(!state.ready)return;AudioStream::Pump();Audio::PumpOnce();if(Hda::state.playbackPrepared){if(Hda::state.testToneEnabled)Hda::FillTestTone();else Hda::SubmitStereo16(Audio::state.mix,Audio::FramesPerBuffer);if(!Hda::state.playbackRunning)Hda::StartPlayback();speakerGate(false);}else if(Hda::state.testToneEnabled&&!Audio::state.muted)speakerTone(1000);else speakerGate(false);state.pumps++;}}
