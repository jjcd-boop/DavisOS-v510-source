#include "SystemServices.hpp"
namespace Davis::SystemServices {
State state{};
void Init(){state={};state.ready=true;}
bool Register(u32 sid,u64 owner,u64 ep){if(!state.ready||!sid||!owner||!ep)return false;for(auto&e:state.entries)if(e.active&&e.serviceId==sid)return false;for(auto&e:state.entries)if(!e.active){e={sid,ep,owner,true};return true;}return false;}
u64 Lookup(u32 sid){for(auto&e:state.entries)if(e.active&&e.serviceId==sid)return e.endpointId;return 0;}
void UnregisterOwner(u64 owner){for(auto&e:state.entries)if(e.active&&e.owner==owner)e={};}
}
