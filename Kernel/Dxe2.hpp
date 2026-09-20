#pragma once
#include "Dxe.hpp"
namespace Davis::Dxe2 {
static constexpr u32 Magic=0x32455844u; // DXE2
static constexpr u16 AbiMajor=2, AbiMinor=0;
static constexpr u32 MaxSegments=8, MaxImports=64;
enum SegmentFlags:u32 { Read=1u<<0, Write=1u<<1, Execute=1u<<2 };
enum Capability:u64 {
 CapWindow=1ull<<0, CapFileRead=1ull<<1, CapFileWrite=1ull<<2, CapNetwork=1ull<<3, CapIpc=1ull<<4,
 // Privileged user-space roles. Executables cannot grant these to themselves;
 // SecurityPolicy only grants them to a trusted launcher-selected domain.
 CapServiceHost=1ull<<16, CapDriverIo=1ull<<17, CapIrq=1ull<<18, CapDma=1ull<<19
};
static constexpr u64 KnownCapabilities=CapWindow|CapFileRead|CapFileWrite|CapNetwork|CapIpc|CapServiceHost|CapDriverIo|CapIrq|CapDma;
#pragma pack(push,1)
struct Header {
 Dxe::Header base;
 u64 appId; u32 appVersionMajor,appVersionMinor;
 u64 capabilities;
 u32 segmentCount,segmentEntryBytes; u64 segmentTableOffset;
 u32 importCount,relocationCount; u64 importTableOffset,relocationTableOffset;
 u64 reserved2[4];
};
struct Segment { u64 fileOffset,fileBytes,virtualAddress,memoryBytes; u32 flags; u32 alignment; u64 reserved; };
// DXE2 imports are stable Davis ABI symbol IDs. The loader resolves each symbol
// and writes its 64-bit ABI value into patchVirtualAddress in writable user data.
struct Import { u32 symbol; u16 abiMajor,abiMinor; u64 patchVirtualAddress; u64 reserved; };
#pragma pack(pop)
enum class ValidateResult:u32 { Ok,NotDxe2,BadHeader,BadAbi,BadCapabilities,BadSegmentTable,BadSegment,OverlappingVirtual,WritableExecutable,EntryNotExecutable,BadImportTable,BadImport,UnsupportedRelocations,ReservedNotZero,ArithmeticOverflow };
ValidateResult Validate(const void*,u64,const Header** out=nullptr);
const char* ResultName(ValidateResult);
}
