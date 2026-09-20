#pragma once
#include "../Base/Types.hpp"
#include "Input.hpp"
namespace Davis::DesktopService {
static constexpr usize MaxBindings=64;
struct Binding { u64 owner,objectId,windowHandle,eventHandle; bool active; };
enum class Capture:u8 { None, Drag, Resize };
struct State { u64 endpointId; u64 frames,eventSequence,focusedObject,capturedObject; i64 lastMouseX,lastMouseY,captureOffsetX,captureOffsetY; bool lastLeft,registered; i32 injectedDx,injectedDy; u8 injectedButtons; bool injectedPointerPending; Capture capture; Binding bindings[MaxBindings]; };
extern State state;
void Init();
void Tick();
void ApplyInjectedInput(Input::State& input,u32 width,u32 height);
void RouteInput(Input::State& input);
bool RouteKey(char c);
}
