#pragma once
#include "CpuContext.hpp"
namespace Davis::TimerFrame {
struct View { CpuContext::Frame* frame; bool fromUser; bool hasHardwareRspSs; bool safeToPreempt; };
inline View Normalize(CpuContext::Frame&f){bool u=CpuContext::FromUser(f);return {&f,u,u,u&&CpuContext::CanonicalRip(f)};}
}
