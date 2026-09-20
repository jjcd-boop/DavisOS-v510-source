#pragma once
#include "BootInfo.hpp"
#include "Graphics.hpp"
#include "Desktop.hpp"
#include "Input.hpp"
namespace Davis::DesktopRuntime {
struct State { Graphics::Surface screen; Graphics::Surface compose; Desktop::State desktop; Input::State input; u64 backBufferPhysical,backBufferPages,pumps,presents; bool backBuffered,ready; };
extern State state;
bool Init(const BootInfo&);
void PumpOnce();
[[noreturn]] void Run();
}
