#include "EmergencyIst.hpp"
#include "KernelMemory.hpp"
namespace Davis::EmergencyIst {
State state{};
bool Prepare(UserMode::GdtState&g,Idt::Table&id,u16 kc){state={};if(!StackGuard::Allocate(state.doubleFault,KernelMemory::allocator,8)||!StackGuard::Allocate(state.nmi,KernelMemory::allocator,4)||!StackGuard::Allocate(state.machineCheck,KernelMemory::allocator,8))return false;g.tss.ist1=state.doubleFault.top;g.tss.ist2=state.nmi.top;g.tss.ist3=state.machineCheck.top;Idt::SetInterruptGate(id,8,DavisDoubleFault,kc,0,1);state.installed=Validate(g,id,kc);return state.installed;}
bool MapInto(Paging::Space&s,Memory::PageAllocator&a){return StackGuard::MapInto(state.doubleFault,s,a)&&StackGuard::MapInto(state.nmi,s,a)&&StackGuard::MapInto(state.machineCheck,s,a);}
bool CanariesIntact(){return StackGuard::CanaryIntact(state.doubleFault)&&StackGuard::CanaryIntact(state.nmi)&&StackGuard::CanaryIntact(state.machineCheck);}
bool Validate(const UserMode::GdtState&g,const Idt::Table&id,u16 kc){auto&df=id.gate[8];auto&n=id.gate[2];auto&m=id.gate[18];return state.doubleFault.ready&&state.nmi.ready&&state.machineCheck.ready&&CanariesIntact()&&g.tss.ist1==state.doubleFault.top&&g.tss.ist2==state.nmi.top&&g.tss.ist3==state.machineCheck.top&&df.selector==kc&&(df.ist&7)==1&&n.selector==kc&&(n.ist&7)==2&&m.selector==kc&&(m.ist&7)==3;}
extern "C" [[noreturn]] void DavisDoubleFaultDispatch(u64*){
#if defined(__x86_64__) && !defined(DAVIS_HOST_TEST)
 __asm__ __volatile__("cli":::"memory");for(;;)__asm__ __volatile__("hlt");
#else
 for(;;){}
#endif
}
}
