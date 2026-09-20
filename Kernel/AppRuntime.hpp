#pragma once
#include "BootInfo.hpp"
namespace Davis::AppRuntime {
enum class Result:u32 { Disabled, NotPresent, MemoryUnavailable, PrepareFailed, MapKernelFailed, PlatformFailed, Returned };
struct Status { Result result; int exitCode; u64 faultVector; u64 syscallCount; bool attempted; bool enteredUser; bool returnedKernel; bool exited; bool faulted; };
extern Status status;
void TryHelloDavis(const BootInfo&);
}
