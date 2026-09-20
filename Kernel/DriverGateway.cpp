#include "DriverGateway.hpp"
#include "DeviceResourceBroker.hpp"
#include "PageTables.hpp"
#include "KernelMemory.hpp"
#include "Ipc.hpp"
#include "../Arch/x86_64/Io.hpp"
namespace Davis::DriverGateway {
State state{};
static bool add(u64 a,u64 b,u64&out){out=a+b;return out>=a;}
static const DeviceResourceBroker::Resource* owned(const Process::Image&p,u64 id,DeviceResourceBroker::Type t,u32 rights){auto*r=DeviceResourceBroker::Find(id);if(!r||r->type!=t||r->ownerProcessId!=p.id||!DeviceResourceBroker::OwnedBy(id,p.id,rights))return nullptr;return r;}
void Init(){state={};state.ready=true;}
bool MapMmio(const Process::Image&p,u64 id,u64*out){
 if(!state.ready||!out||!p.addressSpaceCr3)return false;auto*r=owned(p,id,DeviceResourceBroker::Type::Mmio,DeviceResourceBroker::RightMap);if(!r)return false;
 for(auto&m:state.mappings)if(m.active&&m.processId==p.id&&m.resourceId==id){*out=m.userBase+(r->base&4095);return true;}
 u64 phys=r->base&~4095ull,delta=r->base-phys,span=0;if(!add(delta,r->length,span)||span>0x10000000ull)return false;u64 pages=(span+4095)/4096;if(!pages)return false;
 u64 va=UserMmioBase;for(;;){u64 bytes=pages*4096,end=0;if(!add(va,bytes,end)||end>UserMmioLimit)return false;bool collision=false;for(auto&m:state.mappings)if(m.active&&m.processId==p.id){u64 me=m.userBase+m.pages*4096;if(va<me&&m.userBase<end){va=me;collision=true;break;}}if(!collision)break;}
 Paging::Space s{p.addressSpaceCr3,0,true};for(u64 i=0;i<pages;i++){auto q=Paging::Map4K(s,KernelMemory::allocator,va+i*4096,phys+i*4096,Vm::Writable|Vm::NoExecute);if(q!=Paging::Result::Ok)return false;}
 for(auto&m:state.mappings)if(!m.active){m={p.id,id,va,pages,p.addressSpaceCr3,true};*out=va+delta;return true;}return false;
}
bool PortRead(const Process::Image&p,u64 id,u64 off,u32 width,u32*out){if(!out)return false;auto*r=owned(p,id,DeviceResourceBroker::Type::IoPort,DeviceResourceBroker::RightMap);u64 end=0;if(!r||!add(off,width,end)||end>r->length||r->base+off>0xffff)return false;u16 port=(u16)(r->base+off);if(width==1)*out=Io::In8(port);else if(width==2)*out=Io::In16(port);else if(width==4)*out=Io::In32(port);else return false;return true;}
bool PortWrite(const Process::Image&p,u64 id,u64 off,u32 width,u32 v){auto*r=owned(p,id,DeviceResourceBroker::Type::IoPort,DeviceResourceBroker::RightMap);u64 end=0;if(!r||!add(off,width,end)||end>r->length||r->base+off>0xffff)return false;u16 port=(u16)(r->base+off);if(width==1)Io::Out8(port,(u8)v);else if(width==2)Io::Out16(port,(u16)v);else if(width==4)Io::Out32(port,v);else return false;return true;}
bool AllocateDma(const Process::Image&p,u64 bytes,u64 alignment,u64*outUser,u64*outPhysical){
 if(!state.ready||!outUser||!outPhysical||!p.addressSpaceCr3||!bytes||bytes>0x01000000ull)return false;
 if(alignment<4096)alignment=4096;if(alignment>0x200000ull||(alignment&(alignment-1)))return false;
 auto d=Memory::AllocDma(KernelMemory::allocator,bytes,alignment);if(!d.physical)return false;
 u64 va=0x0000000070000000ull;u64 limit=0x0000000078000000ull;
 for(;;){u64 end=va+d.pages*4096;if(end<va||end>limit){Memory::FreeDma(KernelMemory::allocator,d);return false;}bool hit=false;for(auto&m:state.dma)if(m.active&&m.processId==p.id){u64 me=m.userBase+m.pages*4096;if(va<me&&m.userBase<end){va=me;hit=true;break;}}if(!hit)break;}
 Paging::Space sp{p.addressSpaceCr3,0,true};u64 mapped=0;for(;mapped<d.pages;mapped++){if(Paging::Map4K(sp,KernelMemory::allocator,va+mapped*4096,d.physical+mapped*4096,Vm::Writable|Vm::NoExecute)!=Paging::Result::Ok)break;}
 if(mapped!=d.pages){if(mapped)Paging::UnmapRegion(sp,va,mapped,true);Memory::FreeDma(KernelMemory::allocator,d);return false;}
 for(auto&m:state.dma)if(!m.active){m={p.id,va,d.pages,p.addressSpaceCr3,d,true};*outUser=va;*outPhysical=d.physical;return true;}Paging::UnmapRegion(sp,va,d.pages,true);Memory::FreeDma(KernelMemory::allocator,d);return false;
}
bool FreeDma(const Process::Image&p,u64 user){for(auto&m:state.dma)if(m.active&&m.processId==p.id&&m.userBase==user){Paging::Space sp{m.addressSpaceCr3,m.pages,true};Paging::UnmapRegion(sp,m.userBase,m.pages,true);auto d=m.buffer;bool ok=Memory::FreeDma(KernelMemory::allocator,d);m={};return ok;}return false;}
bool BindIrq(const Process::Image&p,u64 id,u64 endpoint){auto*r=DeviceResourceBroker::Find(id);if(!r||r->ownerProcessId!=p.id||(r->rights&DeviceResourceBroker::RightSignal)==0||(r->type!=DeviceResourceBroker::Type::Irq&&r->type!=DeviceResourceBroker::Type::Msi)||!endpoint)return false;for(auto&b:state.irqs)if(b.active&&b.resourceId==id)return b.processId==p.id&&b.endpointId==endpoint;for(auto&b:state.irqs)if(!b.active){b={p.id,id,endpoint,0,0,true};return true;}return false;}
void DeliverIrq(u64 irq){for(auto&b:state.irqs)if(b.active){auto*r=DeviceResourceBroker::Find(b.resourceId);if(!r||(r->type!=DeviceResourceBroker::Type::Irq&&r->type!=DeviceResourceBroker::Type::Msi)||r->base!=irq)continue;struct Msg{u64 irq,resource;}m{irq,b.resourceId};if(Ipc::Send(0,b.endpointId,0x44524951u,&m,sizeof(m)))b.delivered++;else b.dropped++;}}
void RevokeProcess(u64 pid){if(!pid)return;for(auto&m:state.mappings)if(m.active&&m.processId==pid){Paging::Space s{m.addressSpaceCr3,m.pages,true};Paging::UnmapRegion(s,m.userBase,m.pages,true);m={};}for(auto&m:state.dma)if(m.active&&m.processId==pid){Paging::Space s{m.addressSpaceCr3,m.pages,true};Paging::UnmapRegion(s,m.userBase,m.pages,true);auto d=m.buffer;Memory::FreeDma(KernelMemory::allocator,d);m={};}for(auto&b:state.irqs)if(b.active&&b.processId==pid)b={};}
}
