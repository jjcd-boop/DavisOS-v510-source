#pragma once
#include "PageTables.hpp"
namespace Davis::DeviceMappings {
// Supervisor-only identity mappings required while a user CR3 is active.
// MMIO is writable and NX; it is never exposed with the user bit set.
enum class Result:u32 { Ok,Invalid,MapFailed,VerifyFailed };
Result InstallSupervisorMmio(Paging::Space&,Memory::PageAllocator&,u64 physicalBase,u64 pages);
bool VerifySupervisorMmio(const Paging::Space&,u64 physicalBase,u64 pages);
}
