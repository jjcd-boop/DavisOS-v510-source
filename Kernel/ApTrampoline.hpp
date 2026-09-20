#pragma once
#include "../Base/Types.hpp"
namespace Davis::ApTrampoline {
constexpr u64 HandoffMagic=0x44564150484F4646ull; // DVAPH OFF
constexpr u32 HandoffVersion=2;
#pragma pack(push,1)
struct Handoff {
 u64 magic; u32 version; u32 bytes; u32 apicId; u32 logicalId;
 u64 cr3; u64 stackTop; u64 entryPoint; u64 rendezvousToken; u64 generation;
 u32 prePagingSeal; u64 checksum;
};
#pragma pack(pop)
constexpr u32 HandoffOffsetMagic=0;
constexpr u32 HandoffOffsetVersion=8;
constexpr u32 HandoffOffsetBytes=12;
constexpr u32 HandoffOffsetCr3=24;
constexpr u32 HandoffOffsetStackTop=32;
constexpr u32 HandoffOffsetEntryPoint=40;
constexpr u32 HandoffOffsetToken=48;
constexpr u32 HandoffOffsetGeneration=56;
constexpr u32 HandoffOffsetPrePagingSeal=64;
static_assert(__builtin_offsetof(Handoff,magic)==HandoffOffsetMagic,"handoff magic ABI");
static_assert(__builtin_offsetof(Handoff,version)==HandoffOffsetVersion,"handoff version ABI");
static_assert(__builtin_offsetof(Handoff,bytes)==HandoffOffsetBytes,"handoff bytes ABI");
static_assert(__builtin_offsetof(Handoff,cr3)==HandoffOffsetCr3,"handoff cr3 ABI");
static_assert(__builtin_offsetof(Handoff,stackTop)==HandoffOffsetStackTop,"handoff stack ABI");
static_assert(__builtin_offsetof(Handoff,entryPoint)==HandoffOffsetEntryPoint,"handoff entry ABI");
static_assert(__builtin_offsetof(Handoff,rendezvousToken)==HandoffOffsetToken,"handoff token ABI");
static_assert(__builtin_offsetof(Handoff,generation)==HandoffOffsetGeneration,"handoff generation ABI");
static_assert(__builtin_offsetof(Handoff,prePagingSeal)==HandoffOffsetPrePagingSeal,"handoff seal ABI");
u32 ComputePrePagingSeal(const Handoff&);
u64 ComputeChecksum(const Handoff&);
bool ValidatePrePaging(const Handoff&);
bool Build(Handoff&,u32 apicId,u32 logicalId,u64 cr3,u64 stackTop,u64 entryPoint,u64 token,u64 generation);
bool Validate(const Handoff&);
}
