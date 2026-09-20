#include "XhciDma.hpp"
namespace Davis::Xhci {
static constexpr u32 RingTrbs=256;
void InitDma(DmaState&d){d={};d.commandCycle=true;d.eventCycle=true;}
static u32 ScratchpadCount(const State&s){u32 hi=(s.hcs2>>27)&0x1F,lo=(s.hcs2>>21)&0x1F;return (hi<<5)|lo;}
Trb* CommandTrbs(DmaState&d){return (Trb*)d.commandRing.virtualAddress;}
Trb* EventTrbs(DmaState&d){return (Trb*)d.eventRing.virtualAddress;}
void ReleaseDmaStructures(DmaState&d,Memory::PageAllocator&a){
 for(u32 i=0;i<d.scratchpadCount&&i<256;i++)Memory::FreeDma(a,d.scratchpads[i]);
 Memory::FreeDma(a,d.scratchpadArray);Memory::FreeDma(a,d.erst);Memory::FreeDma(a,d.eventRing);Memory::FreeDma(a,d.commandRing);Memory::FreeDma(a,d.dcbaa);InitDma(d);
}
bool AllocateDmaStructures(DmaState&d,const State&s,Memory::PageAllocator&a){
 InitDma(d);if(!s.capabilitiesValid||!s.maxSlots)return false;
 d.dcbaa=Memory::AllocDma(a,256*8,64);d.commandRing=Memory::AllocDma(a,RingTrbs*sizeof(Trb),64);
 d.eventRing=Memory::AllocDma(a,RingTrbs*sizeof(Trb),64);d.erst=Memory::AllocDma(a,sizeof(ErstEntry),64);
 if(!d.dcbaa.physical||!d.commandRing.physical||!d.eventRing.physical||!d.erst.physical){ReleaseDmaStructures(d,a);return false;}
 auto*cmd=CommandTrbs(d);cmd[RingTrbs-1].parameterLo=(u32)d.commandRing.physical;cmd[RingTrbs-1].parameterHi=(u32)(d.commandRing.physical>>32);cmd[RingTrbs-1].control=(6u<<10)|2u|1u;
 auto*erst=(ErstEntry*)d.erst.virtualAddress;erst[0].ringSegmentBase=d.eventRing.physical;erst[0].ringSegmentSize=RingTrbs;
 d.scratchpadCount=ScratchpadCount(s);if(d.scratchpadCount>256){ReleaseDmaStructures(d,a);return false;}
 if(d.scratchpadCount){d.scratchpadArray=Memory::AllocDma(a,d.scratchpadCount*8,64);if(!d.scratchpadArray.physical){ReleaseDmaStructures(d,a);return false;}auto*arr=(u64*)d.scratchpadArray.virtualAddress;
  for(u32 i=0;i<d.scratchpadCount;i++){d.scratchpads[i]=Memory::AllocDma(a,4096,4096);if(!d.scratchpads[i].physical){ReleaseDmaStructures(d,a);return false;}arr[i]=d.scratchpads[i].physical;}
  ((u64*)d.dcbaa.virtualAddress)[0]=d.scratchpadArray.physical;
 }
 d.valid=ValidateDmaStructures(d,s);if(!d.valid)ReleaseDmaStructures(d,a);return d.valid;
}
bool ValidateDmaStructures(const DmaState&d,const State&s){
 if(!s.capabilitiesValid||!d.dcbaa.physical||!d.commandRing.physical||!d.eventRing.physical||!d.erst.physical)return false;
 if((d.dcbaa.physical&63)||(d.commandRing.physical&63)||(d.eventRing.physical&63)||(d.erst.physical&63))return false;
 if(d.scratchpadCount>256)return false;for(u32 i=0;i<d.scratchpadCount;i++)if(!d.scratchpads[i].physical||(d.scratchpads[i].physical&4095))return false;
 return true;
}
}
