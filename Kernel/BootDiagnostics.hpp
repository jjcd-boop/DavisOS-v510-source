#pragma once
#include "BootInfo.hpp"
#include "Graphics.hpp"
namespace Davis::BootDiagnostics {
struct State { Graphics::Surface screen; u32 row; bool enabled; };
extern State state;
void Begin(const BootInfo&);
void Pass(const char*);
void Note(const char*);
[[noreturn]] void CompleteAndHalt();
}
