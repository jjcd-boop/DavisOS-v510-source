#include "Ipc.hpp"
namespace Davis::Ipc {
State state{};
void Init(){state={};state.nextId=0x8000;}
u64 CreateEndpoint(u64 owner,u32 rights){if(!owner||!(rights&(RightSend|RightReceive)))return 0;for(auto&e:state.endpoints)if(!e.active){e={};e.id=++state.nextId;e.owner=owner;e.rights=rights;e.active=true;return e.id;}return 0;}
bool Send(u64 sender,u64 id,u32 type,const void*data,u32 bytes){if(bytes>MaxPayload||(!data&&bytes))return false;for(auto&e:state.endpoints)if(e.active&&e.id==id){if(!(e.rights&RightReceive)||e.count>=8)return false;auto&m=e.queue[e.tail];m={};m.sender=sender;m.type=type;m.bytes=bytes;m.active=true;for(u32 i=0;i<bytes;i++)m.payload[i]=((const u8*)data)[i];e.tail=(e.tail+1)%8;e.count++;return true;}return false;}
i64 Receive(u64 owner,u64 id,u64*sender,u32*type,void*data,u32 cap){for(auto&e:state.endpoints)if(e.active&&e.id==id&&e.owner==owner){if(!(e.rights&RightReceive))return -1;if(!e.count)return 0;auto&m=e.queue[e.head];if(cap<m.bytes)return -2;if(sender)*sender=m.sender;if(type)*type=m.type;for(u32 i=0;i<m.bytes;i++)((u8*)data)[i]=m.payload[i];u32 n=m.bytes;m={};e.head=(e.head+1)%8;e.count--;return n;}return -1;}

u64 OwnerOf(u64 endpointId){for(usize i=0;i<MaxEndpoints;i++)if(state.endpoints[i].active&&state.endpoints[i].id==endpointId)return state.endpoints[i].owner;return 0;}
void RevokeProcess(u64 owner){for(auto&e:state.endpoints)if(e.active&&e.owner==owner)e={};}
}
