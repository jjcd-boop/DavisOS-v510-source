#include "DesktopRuntime.hpp"
#include "DesktopService.hpp"
#include "KernelMemory.hpp"
#include "PhysicalMemory.hpp"
#include "../Arch/x86_64/Cpu.hpp"
#include "UsbRuntime.hpp"
#include "ServiceSupervisor.hpp"
#include "DiagnosticDump.hpp"
#include "DxeServices.hpp"
namespace Davis::DesktopRuntime {
State state{};
static void CopyRect(i64 x,i64 y,i64 w,i64 h){
 if(!state.backBuffered)return;if(x<0){w+=x;x=0;}if(y<0){h+=y;y=0;}if(x+w>(i64)state.screen.fb.width)w=state.screen.fb.width-x;if(y+h>(i64)state.screen.fb.height)h=state.screen.fb.height-y;if(w<=0||h<=0)return;
 for(i64 yy=0;yy<h;yy++){auto*s=(const u32*)(uptr)(state.compose.fb.base+(uptr)(y+yy)*state.compose.fb.pixelsPerScanLine*4)+x;auto*d=(u32*)(uptr)(state.screen.fb.base+(uptr)(y+yy)*state.screen.fb.pixelsPerScanLine*4)+x;for(i64 xx=0;xx<w;xx++)d[xx]=s[xx];}
}
static void DrawCursor(i64 x,i64 y){auto&f=state.screen.fb;auto put=[&](i64 px,i64 py,u32 c){if(px>=0&&py>=0&&px<(i64)f.width&&py<(i64)f.height)*((u32*)(uptr)(f.base+(uptr)py*f.pixelsPerScanLine*4)+px)=c;};for(i64 yy=0;yy<18;yy++)for(i64 xx=0;xx<3;xx++)put(x+xx,y+yy,0x00FFFFFF);for(i64 yy=0;yy<3;yy++)for(i64 xx=0;xx<12;xx++)put(x+xx,y+yy,0x00FFFFFF);for(i64 yy=3;yy<13;yy++)for(i64 xx=3;xx<6;xx++)put(x+xx,y+yy,0x00000000);}
static void Present(){
 if(!state.backBuffered)return;
 auto&src=state.compose.fb; auto&dst=state.screen.fb;
 // Copy complete scanlines only after composition is finished. The user never sees
 // partially constructed windows/cursors, eliminating the old draw-as-we-go flicker.
 for(u32 y=0;y<dst.height;y++){
  auto*s=(const u32*)(uptr)(src.base+(uptr)y*src.pixelsPerScanLine*4);
  auto*d=(u32*)(uptr)(dst.base+(uptr)y*dst.pixelsPerScanLine*4);
  for(u32 x=0;x<dst.width;x++)d[x]=s[x];
 }
 state.presents++;
}
bool Init(const BootInfo&b){
 state={};if(!b.framebuffer.base||!b.framebuffer.width||!b.framebuffer.height)return false;
 state.screen={b.framebuffer}; state.compose=state.screen;
 // A shadow framebuffer makes each desktop update atomic from the composer's point of view.
 // Use width as the shadow stride so firmware padding never wastes RAM.
 u64 pixels=(u64)b.framebuffer.width*b.framebuffer.height,bytes=pixels*4,pages=(bytes+4095)/4096;
 if(KernelMemory::ready){u64 pa=Memory::AllocPages(KernelMemory::allocator,pages,1);if(pa){state.backBufferPhysical=pa;state.backBufferPages=pages;state.compose.fb.base=(uptr)pa;state.compose.fb.pixelsPerScanLine=b.framebuffer.width;state.backBuffered=true;}}
 Desktop::Init(state.desktop);WindowManager::Init(state.desktop.wm,state.screen.fb.width,state.screen.fb.height);Input::Init(state.input,state.screen.fb.width,state.screen.fb.height);state.ready=true;return true;
}
void PumpOnce(){
 if(!state.ready)return;
 ServiceSupervisor::Tick();ServiceSupervisor::Beat(ServiceSupervisor::Service::Desktop);ServiceSupervisor::Beat(ServiceSupervisor::Service::Input);
 i64 oldX=state.input.mouseX,oldY=state.input.mouseY;bool oldL=state.input.left,oldR=state.input.right,oldM=state.input.middle;u64 oldReports=UsbRuntime::state.reports,oldEvents=UsbRuntime::state.events,oldKeys=state.input.keyEvents;
 Input::Poll(state.input,state.screen.fb.width,state.screen.fb.height);
 bool diagnosticClick=Input::Clicked(state.input)||Input::Released(state.input);
 if(diagnosticClick){DiagnosticDump::Event("INPUT","button transition after Poll",state.input.mouseX,state.input.mouseY,state.input.left,state.input.mousePackets);DiagnosticDump::Snapshot("PRE_DESKTOP_SERVICE");}
 DesktopService::Tick();DesktopService::ApplyInjectedInput(state.input,state.screen.fb.width,state.screen.fb.height);DesktopService::RouteInput(state.input);
 if(diagnosticClick){DiagnosticDump::Event("SERVICE","DesktopService complete");}
 Desktop::Update(state.desktop,state.input,state.screen);
 if(diagnosticClick){DiagnosticDump::Event("DESKTOP","Desktop::Update complete");DiagnosticDump::Snapshot("POST_DESKTOP_UPDATE");}
 bool moved=(oldX!=state.input.mouseX)||(oldY!=state.input.mouseY);bool buttons=(oldL!=state.input.left)||(oldR!=state.input.right)||(oldM!=state.input.middle)||Input::Clicked(state.input)||Input::Released(state.input);bool keys=oldKeys!=state.input.keyEvents;
 bool manipulating=(state.desktop.wm.dragging>=0||state.desktop.wm.resizing>=0);
 // A held title bar used to force a complete 1920x1080 redraw on *every* pump,
 // even when no new pointer packet arrived. Redraw manipulation frames only when
 // the pointer actually moved; button/key/USB transitions still redraw immediately.
 static u64 lastDxePresents=0;u64 dxePresents=0;for(auto&w:DxeServices::state.windows)if(w.active)dxePresents+=w.presents;bool dxeChanged=dxePresents!=lastDxePresents;lastDxePresents=dxePresents;
 bool gameFrame=state.desktop.wm.windows[WindowManager::PlaneShooter].visible&&!state.desktop.wm.windows[WindowManager::PlaneShooter].minimized&&((state.pumps%24)==0); bool scene=(state.pumps==0)||dxeChanged||buttons||keys||(oldReports!=UsbRuntime::state.reports)||(oldEvents!=UsbRuntime::state.events)||Input::Clicked(state.input)||(manipulating&&moved)||gameFrame;
 if(scene){
  if(diagnosticClick){DiagnosticDump::Event("DRAW","Desktop::Draw begin");}
  Desktop::Draw(state.desktop,state.backBuffered?state.compose:state.screen,state.input);
  if(diagnosticClick){DiagnosticDump::Event("DRAW","Desktop::Draw complete");}
  if(state.backBuffered){if(diagnosticClick){DiagnosticDump::Event("PRESENT","full Present begin");}Present();if(diagnosticClick){DiagnosticDump::Event("PRESENT","full Present complete",state.presents);}}
  DrawCursor(state.input.mouseX,state.input.mouseY);
  if(diagnosticClick){DiagnosticDump::Event("CURSOR","DrawCursor complete");DiagnosticDump::Snapshot("PUMP_CLICK_COMPLETE");}
 }
 else if(moved&&state.backBuffered){CopyRect(oldX-1,oldY-1,15,21);DrawCursor(state.input.mouseX,state.input.mouseY);}
 else if(moved){Desktop::Draw(state.desktop,state.screen,state.input);}
 state.pumps++;
}
static void RuntimeHeartbeat(){
 // v1.55 liveness marker: tiny top-right square toggles without involving any service.
 // If it stops changing, the desktop execution path itself has stalled.
 if(!state.screen.fb.base||!state.screen.fb.width||!state.screen.fb.height)return;
 if((state.pumps&0xFFu)!=0)return;
 const u32 c=(state.pumps&0x100u)?0x0000FF00u:0x00202020u;
 const u32 w=state.screen.fb.width<10?state.screen.fb.width:10;
 const u32 h=state.screen.fb.height<10?state.screen.fb.height:10;
 const u32 x0=state.screen.fb.width-w;
 for(u32 y=0;y<h;y++){u32*d=(u32*)(uptr)(state.screen.fb.base+(uptr)y*state.screen.fb.pixelsPerScanLine*4);for(u32 x=x0;x<state.screen.fb.width;x++)d[x]=c;}
}
[[noreturn]] void Run(){
 // ISOLATION INVARIANT: the desktop owns presentation/input only. Never pump
 // audio, network, storage, USB enumeration, clocks, or applications here.
 for(;;){PumpOnce();RuntimeHeartbeat();for(u64 n=0;n<300000;++n)Cpu::Pause();}
}
}
