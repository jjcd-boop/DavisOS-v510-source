#include "XhciController.hpp"
#include "../Arch/x86_64/Io.hpp"
namespace Davis::Xhci {
static constexpr u32 USBCMD=0x00, USBSTS=0x04, CRCR=0x18, DCBAAP=0x30, CONFIG=0x38;
static constexpr u32 CMD_RUN=1u<<0,CMD_HCRST=1u<<1,STS_HCH=1u<<0,STS_CNR=1u<<11;
static inline volatile u32* R32(const State&s,u32 o){return (volatile u32*)(uptr)(s.mmio+s.capLength+o);}
static inline volatile u64* R64(const State&s,u32 o){return (volatile u64*)(uptr)(s.mmio+s.capLength+o);}
static inline void W64split(const State&s,u32 o,u64 v){volatile u32*p=(volatile u32*)(uptr)(s.mmio+s.capLength+o);p[0]=(u32)v;__asm__ __volatile__("mfence" ::: "memory");p[1]=(u32)(v>>32);__asm__ __volatile__("mfence" ::: "memory");}
static inline u64 R64split(const State&s,u32 o){volatile u32*p=(volatile u32*)(uptr)(s.mmio+s.capLength+o);u32 lo=p[0],hi=p[1];return ((u64)hi<<32)|lo;}
static u32 PciRead(const State&s,u8 off){u32 a=0x80000000u|((u32)s.bus<<16)|((u32)s.device<<11)|((u32)s.function<<8)|(off&0xFC);Io::Out32(0xCF8,a);return Io::In32(0xCFC);}
static void PciWrite(const State&s,u8 off,u32 v){u32 a=0x80000000u|((u32)s.bus<<16)|((u32)s.device<<11)|((u32)s.function<<8)|(off&0xFC);Io::Out32(0xCF8,a);Io::Out32(0xCFC,v);}
static bool WaitBits(volatile u32*r,u32 mask,bool set,u32 n,u32&last){while(n--){last=*r;if(((last&mask)!=0)==set)return true;__asm__ __volatile__("pause" ::: "memory");}return false;}
void InitController(ControllerState&c){c={};c.result=StartResult::InvalidState;}
StartResult StartController(State&s,DmaState&d,ControllerState&c,u32 spins){InitController(c);if(!s.capabilitiesValid||!d.valid)return c.result=StartResult::InvalidState;
 // Enable PCI memory decoding/bus mastering before programming DMA-backed xHCI registers.
 u32 pc=PciRead(s,0x04),want=pc|0x6u;PciWrite(s,0x04,want);u32 got=PciRead(s,0x04);c.pciCommand=got;if((got&0x6u)!=0x6u)return c.result=StartResult::PciEnableFailed;c.pciEnabled=true;
 if(!RequestFirmwareHandoff(s,spins))return c.result=StartResult::FirmwareHandoffFailed;c.handoff=true;
 volatile u32*cmd=R32(s,USBCMD);volatile u32*sts=R32(s,USBSTS);
 *cmd &= ~CMD_RUN;if(!WaitBits(sts,STS_HCH,true,spins,c.lastUsbSts))return c.result=StartResult::ControllerNotHalted;c.halted=true;
 *cmd |= CMD_HCRST;if(!WaitBits(cmd,CMD_HCRST,false,spins,c.lastUsbSts))return c.result=StartResult::ResetTimeout;c.reset=true;
 if(!WaitBits(sts,STS_CNR,false,spins,c.lastUsbSts))return c.result=StartResult::NotReady;c.cnrClear=true;c.pageSize=*R32(s,0x08);
 if(!(c.pageSize&1u)||!ValidateDmaStructures(d,s))return c.result=StartResult::DmaInvalid;c.dmaValid=true;
 *R64(s,DCBAAP)=d.dcbaa.physical;__asm__ __volatile__("mfence" ::: "memory");c.dcbaaOk=(((*R64(s,DCBAAP))&~63ull)==(d.dcbaa.physical&~63ull));
 c.crcrExpected=d.commandRing.physical|1ull;
 // CRCR is a 64-bit operational register. Some Intel controllers are more reliable
 // when programmed as ordered 32-bit MMIO halves, so use that form and verify the
 // command-ring pointer bits separately from RCS/CRR control/status bits.
 W64split(s,CRCR,c.crcrExpected);c.crcrReadback=R64split(s,CRCR);
 c.crcrOk=((c.crcrReadback&~63ull)==(d.commandRing.physical&~63ull));
 if(!c.crcrOk){ // one retry after a full barrier; do not start with an unknown CRCR.
   __asm__ __volatile__("mfence" ::: "memory");W64split(s,CRCR,c.crcrExpected);c.crcrReadback=R64split(s,CRCR);
   c.crcrOk=((c.crcrReadback&~63ull)==(d.commandRing.physical&~63ull));
 }
 u64 rt=s.mmio+(s.rtsoff&~0x1Fu);volatile u32*erstsz=(volatile u32*)(uptr)(rt+0x20+0x08);volatile u64*erstba=(volatile u64*)(uptr)(rt+0x20+0x10);volatile u64*erdp=(volatile u64*)(uptr)(rt+0x20+0x18);
 *erstsz=1;*erstba=d.erst.physical;*erdp=d.eventRing.physical;__asm__ __volatile__("mfence" ::: "memory");c.erstOk=(*erstsz==1&&((*erstba)&~63ull)==(d.erst.physical&~63ull));
 *R32(s,CONFIG)=s.maxSlots;c.programmed=c.dcbaaOk&&c.crcrOk&&c.erstOk;if(!c.programmed)return c.result=StartResult::RegisterReadbackFailed;
 *cmd|=CMD_RUN;c.runSet=true;c.lastUsbCmd=*cmd;if(!WaitBits(sts,STS_HCH,false,spins,c.lastUsbSts))return c.result=StartResult::RunTimeout;c.running=true;return c.result=StartResult::Ok;
}
}
