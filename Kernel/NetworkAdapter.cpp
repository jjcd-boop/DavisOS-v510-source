#include "NetworkAdapter.hpp"
#include "E1000.hpp"
#include "Rtl8168.hpp"
namespace Davis::NetworkAdapter {
State state{};
static void identify(const Pci::State&p){for(u32 i=0;i<p.count;i++){const auto&d=p.devices[i];if(d.classCode!=0x02||d.subclass!=0x00)continue;state.controllerFound=true;state.vendor=d.vendor;state.device=d.device;state.bus=d.bus;state.slot=d.slot;state.function=d.function;if(Rtl8168::IsSupported(d)){state.backend=Backend::RealtekRtl8168;return;}if(E1000::IsSupported(d)){state.backend=Backend::IntelE1000;return;}if(state.backend==Backend::None)state.backend=Backend::UnsupportedEthernet;}}
bool Init(const Pci::State&p,Memory::PageAllocator&a){state={};identify(p);if(state.backend==Backend::IntelE1000)state.ready=E1000::Init(p,a);else if(state.backend==Backend::RealtekRtl8168)state.ready=Rtl8168::Init(p,a);state.link=state.ready&&((state.backend==Backend::IntelE1000)?E1000::state.link:(state.backend==Backend::RealtekRtl8168?Rtl8168::state.link:false));return state.ready;}
bool Send(const void*f,u32 n){bool ok=state.backend==Backend::IntelE1000?E1000::Send(f,n):state.backend==Backend::RealtekRtl8168?Rtl8168::Send(f,n):false;if(ok)state.tx++;else state.txDrops++;return ok;}
bool Receive(Net::Packet&o){bool ok=state.backend==Backend::IntelE1000?E1000::Receive(o):state.backend==Backend::RealtekRtl8168?Rtl8168::Receive(o):false;if(ok)state.rx++;return ok;}
Net::Mac Mac(){return state.backend==Backend::IntelE1000?E1000::state.mac:state.backend==Backend::RealtekRtl8168?Rtl8168::state.mac:Net::Mac{};}
const char* BackendName(){switch(state.backend){case Backend::IntelE1000:return "INTEL E1000 FAMILY";case Backend::RealtekRtl8168:return "REALTEK RTL8168/8111";case Backend::UnsupportedEthernet:return "ETHERNET CHIPSET UNSUPPORTED";default:return "NO ETHERNET BACKEND";}}
}
