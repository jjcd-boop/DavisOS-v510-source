#pragma once
#include "Xhci.hpp"
#include "Dma.hpp"
namespace Davis::Xhci {
struct Trb { u32 parameterLo,parameterHi,status,control; };
struct ErstEntry { u64 ringSegmentBase; u32 ringSegmentSize; u32 reserved; };
struct DmaState {
 Memory::DmaBuffer dcbaa, commandRing, eventRing, erst, scratchpadArray;
 Memory::DmaBuffer scratchpads[256]; u32 scratchpadCount;
 u32 commandIndex,eventIndex; bool commandCycle,eventCycle; bool valid;
};
void InitDma(DmaState&);
bool AllocateDmaStructures(DmaState&,const State&,Memory::PageAllocator&);
void ReleaseDmaStructures(DmaState&,Memory::PageAllocator&);
bool ValidateDmaStructures(const DmaState&,const State&);
Trb* CommandTrbs(DmaState&); Trb* EventTrbs(DmaState&);
}
