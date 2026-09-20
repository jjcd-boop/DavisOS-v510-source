#include "BootDiagnostics.hpp"
#include "../Arch/x86_64/Cpu.hpp"
namespace Davis::BootDiagnostics {
State state{};
static void line(const char* mark,const char* text,u32 color){if(!state.enabled)return;auto& s=state.screen;Graphics::Text(s,24,58+(i64)state.row*18,mark,color,1);Graphics::Text(s,88,58+(i64)state.row*18,text,Graphics::RGB(235,240,246),1);state.row++;}
void Begin(const BootInfo& b){state={};if(!b.diagnosticBootRequested||!b.framebuffer.base||!b.framebuffer.width||!b.framebuffer.height)return;state.enabled=true;state.screen={b.framebuffer};Graphics::Clear(state.screen,Graphics::RGB(12,18,28));Graphics::Text(state.screen,24,18,"DAVIS OS HARDWARE BRING-UP v1.50",Graphics::RGB(120,210,255),2);Graphics::Text(state.screen,24,44,"Diagnostic boot requested from DAVIS/DIAGNOSTIC_BOOT.TXT",Graphics::RGB(190,200,215),1);}
void Pass(const char* t){line("[PASS]",t,Graphics::RGB(90,235,130));}
void Note(const char* t){line("[....]",t,Graphics::RGB(255,205,90));}
[[noreturn]] void CompleteAndHalt(){line("[PASS]","Bring-up sequence completed",Graphics::RGB(90,235,130));line("[STOP]","Diagnostic mode intentionally halted",Graphics::RGB(255,205,90));line("      ","Power off and remove DIAGNOSTIC_BOOT.TXT for normal boot",Graphics::RGB(235,240,246));Cpu::HaltForever();}
}
