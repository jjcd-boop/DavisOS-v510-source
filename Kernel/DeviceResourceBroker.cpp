#include "DeviceResourceBroker.hpp"
#include "Dxe2.hpp"
#include "DriverManager.hpp"
namespace Davis::DeviceResourceBroker {
State state{};
static bool addOk(u64 a,u64 b,u64&out){out=a+b;return out>=a;}
static bool typeRights(Type t,u32 r){
 const u32 known=RightInspect|RightMap|RightSignal|RightDma;if(!r||(r&~known))return false;
 switch(t){case Type::Mmio:return (r&~(RightInspect|RightMap))==0;case Type::IoPort:return (r&~(RightInspect|RightMap))==0;case Type::Irq:case Type::Msi:return (r&~(RightInspect|RightSignal))==0;case Type::DmaWindow:return (r&~(RightInspect|RightDma))==0;default:return false;}
}
void Init(){state={};state.nextId=1;state.ready=true;}
u64 Register(Type t,u64 dev,u64 base,u64 len,u32 rights){
 if(!state.ready||t==Type::None||!dev||!len||!typeRights(t,rights))return 0;u64 end=0;if(!addOk(base,len,end)||end<=base)return 0;
 for(auto&e:state.resources)if(e.active&&e.deviceId==dev&&e.type==t){u64 ee=0;if(!addOk(e.base,e.length,ee))return 0;if(base<ee&&e.base<end)return 0;}
 for(auto&e:state.resources)if(!e.active){u64 id=state.nextId++;if(!id)id=state.nextId++;e={id,t,base,len,rights,dev,0,true};return id;}return 0;
}
const Resource* Find(u64 id){if(!state.ready||!id)return nullptr;for(auto&e:state.resources)if(e.active&&e.id==id)return &e;return nullptr;}
Resource* FindMutable(u64 id){if(!state.ready||!id)return nullptr;for(auto&e:state.resources)if(e.active&&e.id==id)return &e;return nullptr;}
bool Claim(const Process::Image&p,u64 id,u32 rights){
 u64 assigned=DriverManager::AssignedDevice(p.id);if(!assigned)return false;
 if(p.domain!=SecurityDomain::Kind::Driver||!p.dxe2||(p.grantedCapabilities&Dxe2::CapDriverIo)==0||!rights)return false;
 for(auto&e:state.resources)if(e.active&&e.id==id){if(e.deviceId!=assigned)return false;if(e.ownerProcessId||(rights&~e.rights))return false;if((e.type==Type::Irq||e.type==Type::Msi)&&(p.grantedCapabilities&Dxe2::CapIrq)==0)return false;if((e.type==Type::DmaWindow)&&(p.grantedCapabilities&Dxe2::CapDma)==0)return false;e.ownerProcessId=p.id;return true;}return false;
}
bool Release(const Process::Image&p,u64 id){for(auto&e:state.resources)if(e.active&&e.id==id&&e.ownerProcessId==p.id){e.ownerProcessId=0;return true;}return false;}
void RevokeProcess(u64 owner){if(!owner)return;for(auto&e:state.resources)if(e.active&&e.ownerProcessId==owner)e.ownerProcessId=0;}
bool OwnedBy(u64 id,u64 owner,u32 rights){for(auto&e:state.resources)if(e.active&&e.id==id&&e.ownerProcessId==owner&&((e.rights&rights)==rights))return true;return false;}
}
