#pragma once
#include "Process.hpp"
#include "PageTables.hpp"
namespace Davis::UserMemory {
enum class Access:u32 { Read, Write };
enum class Result:u32 { Ok, NullPointer, Overflow, Unmapped, Supervisor, ReadOnly, ExecutableWrite, InvalidProcess };
Result ValidateRange(const Process::Image&,u64 userVa,u64 bytes,Access);
Result CopyFromUser(const Process::Image&,void*kernelDst,u64 userSrc,u64 bytes);
Result CopyToUser(const Process::Image&,u64 userDst,const void*kernelSrc,u64 bytes);
Result CopyStringFromUser(const Process::Image&,char*kernelDst,u64 capacity,u64 userSrc,u64*lengthOut=nullptr);
}
