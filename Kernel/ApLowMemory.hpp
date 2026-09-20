#pragma once
#include "../Base/Types.hpp"
#include "ApTrampoline.hpp"
namespace Davis::ApLowMemory {
constexpr u32 ImageBytes=4096; constexpr u32 HandoffOffset=512;
struct Image{u8 bytes[ImageBytes];u64 physical;bool ready;};
u32 TrampolineBytes();
bool Build(Image&,u64 physical,const ApTrampoline::Handoff&);
bool Validate(const Image&);
const ApTrampoline::Handoff* GetHandoff(const Image&);
}
