#pragma once
#include "BootInfo.hpp"
namespace Davis::Power {
void Init(const BootInfo&);
[[noreturn]] void Shutdown();
}
