#pragma once
#include "Process.hpp"
namespace Davis::Faults {
enum class Kind:u32 { DivideError=0,InvalidOpcode=6,GeneralProtection=13,PageFault=14,Other=255 };
struct Report { Kind kind; u64 errorCode; u64 rip; u64 address; bool fromUser; };
bool IsUserCodeSelector(u64 cs);
void Contain(Process::Image* current,const Report&);
}
