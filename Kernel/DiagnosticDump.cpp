#include "DiagnosticDump.hpp"
#include "DesktopRuntime.hpp"
#include "UsbRuntime.hpp"
#include "StorageService.hpp"
#include "I2cHid.hpp"
#include "KernelMemory.hpp"
namespace Davis::DiagnosticDump {
State state{};
struct Guid{u32 a;u16 b,c;u8 d[8];};
struct Hdr{u64 signature;u32 revision,headerSize,crc32,reserved;};
struct RuntimeServices{Hdr hdr;void*getTime;void*setTime;void*getWakeupTime;void*setWakeupTime;void*setVirtualAddressMap;void*convertPointer;void*getVariable;void*getNextVariableName;u64(__attribute__((ms_abi))*setVariable)(u16*,Guid*,u32,usize,void*);};
static Guid dumpGuid={0xD415D415,0x1420,0x4A42,{0x9A,0x31,0x44,0x41,0x56,0x49,0x53,0x42}};
static u16 dumpName[]={'D','a','v','i','s','D','i','a','g','D','u','m','p',0};
static void ch(char c){if(state.length+1<sizeof(state.text)){state.text[state.length++]=c;state.text[state.length]=0;}}
static void str(const char*s){if(!s)return;while(*s)ch(*s++);}
static void dec(u64 v){char b[24];u32 n=0;if(!v){ch('0');return;}while(v&&n<sizeof(b)){b[n++]=char('0'+v%10);v/=10;}while(n)ch(b[--n]);}
static void hex(u64 v){str("0x");for(int i=15;i>=0;i--){u8 n=(v>>(i*4))&15;ch(n<10?'0'+n:'A'+n-10);}}
static void kv(const char*k,u64 v){str(k);ch('=');dec(v);ch(' ');} static void kvh(const char*k,u64 v){str(k);ch('=');hex(v);ch(' ');}
static void nl(){ch('\r');ch('\n');}
void Event(const char*tag,const char*detail,u64 a,u64 b,u64 c,u64 d){if(!state.enabled)return;str("EV ");dec(++state.sequence);str(" [");str(tag);str("] ");str(detail);str(" a=");dec(a);str(" b=");dec(b);str(" c=");dec(c);str(" d=");dec(d);nl();}
void Snapshot(const char*reason){if(!state.enabled)return;auto&dr=DesktopRuntime::state;auto&in=dr.input;auto&wm=dr.desktop.wm;auto&usb=UsbRuntime::state;
 str("SNAPSHOT ");str(reason);nl();kv("pumps",dr.pumps);kv("presents",dr.presents);kv("backbuf",dr.backBuffered);kvh("fb",dr.screen.fb.base);kv("w",dr.screen.fb.width);kv("h",dr.screen.fb.height);kv("stride",dr.screen.fb.pixelsPerScanLine);nl();
 kv("mx",in.mouseX);kv("my",in.mouseY);kv("L",in.left);kv("R",in.right);kv("M",in.middle);kv("clickLatch",in.pressLatched);kv("releaseLatch",in.releaseLatched);kv("keyEvents",in.keyEvents);kv("mousePackets",in.mousePackets);kv("shift",in.shift);kv("ctrl",in.ctrl);kv("alt",in.alt);nl();
 kv("dragging",(u64)(i64)wm.dragging);kv("resizing",(u64)(i64)wm.resizing);kv("nextZ",wm.nextZ);nl();
 for(u32 i=0;i<WindowManager::Count;i++){auto&w=wm.windows[i];str("WIN ");dec(i);ch(' ');kv("vis",w.visible);kv("min",w.minimized);kv("max",w.maximized);kv("z",w.z);kv("x",w.bounds.x);kv("y",w.bounds.y);kv("ww",w.bounds.w);kv("wh",w.bounds.h);nl();}
 kv("usbReady",usb.ready);kv("usbResult",(u32)usb.result);kv("usbBringup",(u32)usb.bringup);kv("usbPolls",usb.polls);kv("usbEvents",usb.events);kv("usbReports",usb.reports);kv("xhciStart",(u32)usb.controller.result);kv("running",usb.controller.running);kv("runSet",usb.controller.runSet);kv("crcrOk",usb.controller.crcrOk);kvh("crcrExpected",usb.controller.crcrExpected);kvh("crcrReadback",usb.controller.crcrReadback);kvh("usbSts",usb.controller.lastUsbSts);kvh("usbCmd",usb.controller.lastUsbCmd);nl();
 kv("storageReady",StorageService::state.ready);kv("storagePumps",StorageService::state.pumps);kv("i2cStage",(u32)I2cHid::state.stage);kv("kmemReady",KernelMemory::ready);nl();
}
void Persist(const char*checkpoint){if(!state.enabled||!state.runtimeReady)return;Event("PERSIST",checkpoint,state.persists,state.length,0,0);auto*rt=(RuntimeServices*)state.runtimeServices;if(!rt||!rt->setVariable){state.failures++;return;}constexpr u32 attrs=1|2|4;u64 r=rt->setVariable(dumpName,&dumpGuid,attrs,state.length,state.text);if(r==0)state.persists++;else state.failures++;}
void Init(const BootInfo&b){state={};state.enabled=b.extensiveDumpRequested;state.runtimeServices=b.runtimeServices;state.runtimeReady=state.enabled&&state.runtimeServices;if(!state.enabled)return;str("DAVIS OS EXTENSIVE DIAGNOSTIC DUMP v1.42\r\n");str("Persistent journal: UEFI NVRAM -> exported next boot to DAVIS/LASTDUMP.TXT\r\n");kvh("runtimeServices",state.runtimeServices);kvh("rsdp",b.rsdp);kvh("memoryMap",b.memoryMap);kv("memoryMapBytes",b.memoryMapBytes);kv("descriptorSize",b.descriptorSize);kv("safeBoot",b.safeBootRequested);kv("physicalRO",b.physicalStorageReadOnly);nl();Event("BOOT","diagnostic journal initialized");}
}
