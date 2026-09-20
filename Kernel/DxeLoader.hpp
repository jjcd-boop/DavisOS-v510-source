#pragma once
#include "Dxe.hpp"
#include "SecurityDomain.hpp"
#include "Process.hpp"
#include "PageTables.hpp"
namespace Davis::DxeLoader {
struct Loaded { Paging::Space space; Process::Image* process; u64 codeVa,dataVa,stackBase,stackTop; u64 codePages,dataPages,stackPages; bool sharedKernelMappings; };
enum class Result:u32 { Ok,InvalidDxe,NoProcess,OutOfMemory,MapFailed,LayoutOverflow,VerificationFailed };
Result Prepare(Process::Table&,Memory::PageAllocator&,const void*image,u64 bytes,Loaded&);
bool VerifyUserLayout(const Loaded&,const Dxe::Header&);
}

namespace Davis::DxeLoader { Result PrepareDxe2(Process::Table&,Memory::PageAllocator&,const void*,u64,Loaded&); Result PrepareDxe2AsDomain(Process::Table&,Memory::PageAllocator&,const void*,u64,Loaded&,SecurityDomain::Kind); }
