#pragma once
#include "../Base/Types.hpp"
namespace Davis::Sdk {
static constexpr u32 AppAbiMajor=1, AppAbiMinor=0;
using Handle=u64;
enum class Syscall:u64 { Exit=0, Log=1, Yield=2, CreateWindow=16, PresentWindow=17,
 OpenFile=32, ReadFile=33, WriteFile=34, CloseFile=35 };
struct AppInfo { u32 abiMajor,abiMinor; const char* name; };
using Entry=int(*)(const AppInfo*);
}
extern "C" int DavisMain(const Davis::Sdk::AppInfo*);
