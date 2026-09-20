#pragma once
#include "../Base/Types.hpp"
namespace Davis::Dxe {
static constexpr u32 Magic=0x31455844u; // "DXE1" little-endian
static constexpr u16 AbiMajor=1, AbiMinor=0;
static constexpr u64 MinStackBytes=16*1024, MaxStackBytes=8*1024*1024;
enum Flags:u32 { PositionIndependent=1u<<0, UserMode=1u<<1 };
static constexpr u32 KnownFlags=PositionIndependent|UserMode;
#pragma pack(push,1)
struct Header {
 u32 magic; u16 headerBytes; u16 abiMajor; u16 abiMinor; u16 reserved0;
 u32 flags; u64 imageBytes; u64 entryOffset; u64 codeOffset; u64 codeBytes;
 u64 dataOffset; u64 dataBytes; u64 bssBytes; u64 requiredStackBytes;
 u64 reserved[4];
};
#pragma pack(pop)
enum class ValidateResult:u32 { Ok,NullImage,TooSmall,BadMagic,BadHeader,UnsupportedAbi,
 BadFlags,BadImageSize,BadCodeRange,BadDataRange,BadEntry,BadAlignment,OverlappingSegments,
 BadStackSize,ReservedNotZero,ArithmeticOverflow };
ValidateResult Validate(const void* image,u64 bytes,const Header** outHeader=nullptr);
const char* ResultName(ValidateResult);
}
