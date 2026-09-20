#pragma once
#include "Dxe2.hpp"
#include "PageTables.hpp"
namespace Davis::DxeImports {
enum Symbol:u32 { Exit=1, Log=2, Yield=3, WindowCreate=0x100, WindowPresent=0x101, FileOpen=0x200, FileRead=0x201, FileWrite=0x202, FileClose=0x203, ServiceLookup=0x300, IpcCreate=0x301, IpcSend=0x302, IpcReceive=0x303, IpcClose=0x304 };
enum class Result:u32 { Ok,UnknownSymbol,AbiMismatch,CapabilityDenied,BadPatchAddress };
struct Resolution { u64 value; u64 requiredCapability; };
Result ResolveSymbol(u32 symbol,u16 major,u16 minor,Resolution&);
Result Bind(const Dxe2::Header&,const void* image,const Paging::Space&);
}
