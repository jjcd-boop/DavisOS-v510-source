#pragma once
#include "PageTables.hpp"
namespace Davis::AddressSpace {
static constexpr u64 KernelImageBase=0x02000000ull;
// Must match the UEFI loader aperture and physical allocator reservation.
// v1.89: this was stale at 16 MiB while Boot/Main.cpp and PhysicalMemory.cpp
// reserve 64 MiB. Ring-3 CR3s could therefore omit live kernel/BSS pages.
static constexpr u64 KernelImageSpan=64ull*1024*1024;
static constexpr u64 KernelStackVa=0x000000003F000000ull;
static constexpr u64 KernelStackPages=16;
struct State { Paging::Space space; u64 kernelStackPhysical; u64 kernelStackTop; bool ready; bool sharedKernelMappings; };
enum class Result:u32 { Ok,OutOfMemory,MapFailed,Invalid,ContractFailed };
Result Create(State&,Memory::PageAllocator&);
// Installs the supervisor-only mappings every process CR3 must retain while the
// kernel handles traps/IRQs. rsp0Base is identity mapped for the current
// per-process kernel stack. Additional device MMIO contracts are added by the
// owning driver before hardware preemption is enabled.
Result InstallKernelContract(Paging::Space&,Memory::PageAllocator&,u64 rsp0Base,u64 rsp0Pages);
bool VerifyKernelContract(const Paging::Space&,u64 rsp0Base,u64 rsp0Pages);
}
