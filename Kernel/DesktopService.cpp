#include "DesktopService.hpp"
#include "DesktopProtocol.hpp"
#include "DxeServices.hpp"
#include "Ipc.hpp"
#include "ObjectHandles.hpp"
#include "Syscall.hpp"
#include "SystemServices.hpp"
namespace Davis::DesktopService {
State state{};
static bool sendTo(u64 pid,u64 token,u32 type,const void*data,u32 bytes){auto*e=ObjectHandles::Resolve(Syscall::HandlesForTest(),pid,token,ObjectHandles::Type::IpcEndpoint,ObjectHandles::RightReceive);return e&&Ipc::Send(1,e->objectId,type,data,bytes);}
static Binding* bindingByObject(u64 id){for(auto&b:state.bindings)if(b.active&&b.objectId==id)return &b;return nullptr;}
static void bind(u64 owner,u64 object,u64 wh,u64 eh){for(auto&b:state.bindings)if(!b.active){b={owner,object,wh,eh,true};return;}}
static void unbind(u64 object){if(auto*b=bindingByObject(object))*b={};if(state.focusedObject==object)state.focusedObject=0;}
static void focus(u64 object){if(state.focusedObject==object)return;u64 old=state.focusedObject;state.focusedObject=object;for(u32 pass=0;pass<2;pass++){u64 id=pass?object:old;if(auto*b=bindingByObject(id)){DesktopProtocol::Focus e{{DesktopProtocol::Major,DesktopProtocol::Minor,sizeof(e),b->windowHandle,++state.eventSequence},id==object,0};sendTo(b->owner,b->eventHandle,DesktopProtocol::FocusEvent,&e,sizeof(e));}}}
void Init(){state={};state.endpointId=Ipc::CreateEndpoint(1,Ipc::RightReceive);state.registered=state.endpointId&&SystemServices::Register(SystemServices::DesktopCompositor,1,state.endpointId);}
void Tick(){if(!state.registered)return;state.frames++;for(u32 budget=0;budget<8;budget++){u8 payload[Ipc::MaxPayload]{};u64 sender=0;u32 type=0;i64 n=Ipc::Receive(1,state.endpointId,&sender,&type,payload,sizeof(payload));if(n<=0)break;if((u64)n<sizeof(DesktopProtocol::Header))continue;auto*h=(DesktopProtocol::Header*)payload;if(h->major!=DesktopProtocol::Major||h->minor>DesktopProtocol::Minor||h->bytes!=(u32)n)continue;
 if(type==DesktopProtocol::Hello&&n==(i64)sizeof(DesktopProtocol::HelloRequest)){DesktopProtocol::HelloResponse r{{DesktopProtocol::Major,DesktopProtocol::Minor,sizeof(r),0,h->requestId},0,0};sendTo(sender,h->replyHandle,DesktopProtocol::HelloReply,&r,sizeof(r));}
 else if(type==DesktopProtocol::CreateWindow&&n==(i64)sizeof(DesktopProtocol::CreateWindowRequest)){auto*q=(DesktopProtocol::CreateWindowRequest*)payload;DesktopProtocol::CreateWindowResponse r{{DesktopProtocol::Major,DesktopProtocol::Minor,sizeof(r),0,h->requestId},1,0,0};u64 id=DxeServices::CreateWindow(sender,q->x,q->y,q->width,q->height);if(id){u64 wh=ObjectHandles::Create(Syscall::HandlesForTest(),sender,ObjectHandles::Type::Window,id,ObjectHandles::RightPresent|ObjectHandles::RightClose);if(wh){r.status=0;r.windowHandle=wh;bind(sender,id,wh,h->replyHandle);}else DxeServices::DestroyWindow(sender,id);}sendTo(sender,h->replyHandle,DesktopProtocol::CreateWindowReply,&r,sizeof(r));}
 else if(type==DesktopProtocol::InputInject&&n==(i64)sizeof(DesktopProtocol::InputInjection)){auto*q=(DesktopProtocol::InputInjection*)payload;bool trusted=false;for(auto&e:SystemServices::state.entries)if(e.active&&e.serviceId==SystemServices::InputDriver&&e.owner==sender){trusted=true;break;}if(trusted&&q->length<=64){if(q->protocol==2&&q->length>=3){state.injectedButtons=q->data[0];state.injectedDx+=(i8)q->data[1];state.injectedDy+=(i8)q->data[2];state.injectedPointerPending=true;}else if(q->protocol==1&&q->length>=8){static u8 previous[6]{};bool shift=(q->data[0]&0x22)!=0;for(u32 k=2;k<8;k++){u8 usage=q->data[k];if(!usage)continue;bool was=false;for(u32 j=0;j<6;j++)if(previous[j]==usage)was=true;if(was)continue;char c=0;if(usage>=4&&usage<=29)c=(char)((shift?'A':'a')+usage-4);else if(usage>=30&&usage<=38)c=(char)('1'+usage-30);else if(usage==39)c='0';else if(usage==40)c='\n';else if(usage==42)c='\b';else if(usage==44)c=' ';else if(usage==45)c=shift?'_':'-';else if(usage==46)c=shift?'+':'=';else if(usage==54)c=shift?'<':',';else if(usage==55)c=shift?'>':'.';else if(usage==56)c=shift?'?':'/';if(c)RouteKey(c);}for(u32 j=0;j<6;j++)previous[j]=q->data[j+2];}}} else if(type==DesktopProtocol::CloseWindow&&n==(i64)sizeof(DesktopProtocol::CloseWindowRequest)){auto*q=(DesktopProtocol::CloseWindowRequest*)payload;DesktopProtocol::CloseWindowResponse r{{DesktopProtocol::Major,DesktopProtocol::Minor,sizeof(r),0,h->requestId},1,0};auto*e=ObjectHandles::Resolve(Syscall::HandlesForTest(),sender,q->windowHandle,ObjectHandles::Type::Window,ObjectHandles::RightClose);if(e){u64 id=e->objectId;DxeServices::DestroyWindow(sender,id);if(ObjectHandles::Close(Syscall::HandlesForTest(),sender,q->windowHandle)){r.status=0;unbind(id);}}sendTo(sender,h->replyHandle,DesktopProtocol::CloseWindowReply,&r,sizeof(r));}}}
void ApplyInjectedInput(Input::State&in,u32 width,u32 height){if(!state.injectedPointerPending)return;in.prevLeft=in.left;in.mouseX+=state.injectedDx;in.mouseY-=state.injectedDy;if(in.mouseX<0)in.mouseX=0;if(in.mouseY<0)in.mouseY=0;if(width&&in.mouseX>=(i64)width)in.mouseX=width-1;if(height&&in.mouseY>=(i64)height)in.mouseY=height-1;in.left=(state.injectedButtons&1)!=0;in.right=(state.injectedButtons&2)!=0;in.middle=(state.injectedButtons&4)!=0;in.pressLatched=!in.prevLeft&&in.left;in.releaseLatched=in.prevLeft&&!in.left;in.mousePackets++;state.injectedDx=state.injectedDy=0;state.injectedPointerPending=false;}
void RouteInput(Input::State& in){
 if(!state.registered)return;
 i64 dx=in.mouseX-state.lastMouseX,dy=in.mouseY-state.lastMouseY;
 bool press=in.left&&!state.lastLeft,release=!in.left&&state.lastLeft;
 if(press){
  DxeServices::Window* hit=nullptr;
  for(auto&w:DxeServices::state.windows)if(w.active&&in.mouseX>=w.x&&in.mouseX<w.x+w.w&&in.mouseY>=w.y&&in.mouseY<w.y+w.h+32)if(!hit||w.z>hit->z)hit=&w;
  if(hit){
   focus(hit->id);DxeServices::RaiseWindow(hit->owner,hit->id);
   i64 lx=in.mouseX-hit->x,ly=in.mouseY-hit->y;
   if(ly<32&&lx>=hit->w-32){
    if(auto*b=bindingByObject(hit->id)){DesktopProtocol::Close e{{DesktopProtocol::Major,DesktopProtocol::Minor,sizeof(e),b->windowHandle,++state.eventSequence},1,0};sendTo(b->owner,b->eventHandle,DesktopProtocol::CloseEvent,&e,sizeof(e));}
   }else if(ly<32){state.capture=Capture::Drag;state.capturedObject=hit->id;state.captureOffsetX=lx;state.captureOffsetY=ly;}
   else if(lx>=hit->w-14&&ly>=hit->h+18){state.capture=Capture::Resize;state.capturedObject=hit->id;state.captureOffsetX=hit->w-lx;state.captureOffsetY=hit->h+32-ly;}
  }
 }
 if(in.left&&state.capturedObject){if(auto*b=bindingByObject(state.capturedObject)){if(auto*w=DxeServices::MutableWindow(b->owner,b->objectId)){
  if(state.capture==Capture::Drag){i64 nx=in.mouseX-state.captureOffsetX,ny=in.mouseY-state.captureOffsetY;if(ny<56)ny=56;DxeServices::MoveWindow(b->owner,b->objectId,nx,ny);}
  else if(state.capture==Capture::Resize){i64 nw=in.mouseX-w->x+state.captureOffsetX,nh=in.mouseY-w->y-32+state.captureOffsetY;if(nw<64)nw=64;if(nh<48)nh=48;if(nw>(i64)DxeServices::MaxSurfaceWidth)nw=DxeServices::MaxSurfaceWidth;if(nh>(i64)DxeServices::MaxSurfaceHeight)nh=DxeServices::MaxSurfaceHeight;DxeServices::ResizeWindow(b->owner,b->objectId,nw,nh);}
 }}}
 if(release){state.capture=Capture::None;state.capturedObject=0;}
 if(auto*b=bindingByObject(state.focusedObject)){
  if(dx||dy||press||release){auto*w=DxeServices::FindWindow(b->owner,b->objectId);if(w){u32 f=(in.left?DesktopProtocol::PointerLeft:0)|(press?DesktopProtocol::PointerPressed:0)|(release?DesktopProtocol::PointerReleased:0);DesktopProtocol::Pointer e{{DesktopProtocol::Major,DesktopProtocol::Minor,sizeof(e),b->windowHandle,++state.eventSequence},(i32)(in.mouseX-w->x),(i32)(in.mouseY-w->y-32),(i32)dx,(i32)dy,f,0};sendTo(b->owner,b->eventHandle,DesktopProtocol::PointerEvent,&e,sizeof(e));}}
  // v1.40: The native Desktop owns the PS/2 character queue.  DesktopService must
  // not drain Input::PopChar() before Desktop::Update() can deliver characters to
  // Writer/Terminal/Math. DXE keyboard delivery will move to a fan-out event queue.
 }
 state.lastMouseX=in.mouseX;state.lastMouseY=in.mouseY;state.lastLeft=in.left;
}
bool RouteKey(char c){if(!state.registered||!state.focusedObject||!c)return false;if(auto*b=bindingByObject(state.focusedObject)){DesktopProtocol::Key e{{DesktopProtocol::Major,DesktopProtocol::Minor,sizeof(e),b->windowHandle,++state.eventSequence},(u32)(u8)c,0};return sendTo(b->owner,b->eventHandle,DesktopProtocol::KeyEvent,&e,sizeof(e));}return false;}

}