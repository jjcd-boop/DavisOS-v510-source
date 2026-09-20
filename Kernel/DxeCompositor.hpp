#pragma once
#include "../Base/Types.hpp"
#include "Graphics.hpp"
namespace Davis::DxeCompositor {
struct Stats { u64 visibleWindows, presentedWindows, totalPresents; };
Stats Draw(Graphics::Surface& g, u32 accent, u32 panel, u32 text, u32 muted);
}
