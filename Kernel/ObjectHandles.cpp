#include "ObjectHandles.hpp"
namespace Davis::ObjectHandles {
void Init(Table&t){t={};t.nextNonce=0xD4515001ull;}
static u64 makeToken(u64 nonce,usize slot){return ((nonce&0x0000FFFFFFFFFFFFull)<<16)|((u64)slot+1);}
u64 Create(Table&t,u64 owner,Type type,u64 objectId,u32 rights){if(!owner||type==Type::None)return 0;for(usize i=0;i<MaxHandles;i++)if(!t.entries[i].active){u64 n=++t.nextNonce;u64 tok=makeToken(n,i);t.entries[i]={tok,owner,type,objectId,rights,true};return tok;}return 0;}
const Entry* Resolve(const Table&t,u64 owner,u64 token,Type expected,u32 rights){if(!owner||!token)return nullptr;usize slot=(usize)((token&0xffffull)-1);if(slot>=MaxHandles)return nullptr;const auto&e=t.entries[slot];if(!e.active||e.token!=token||e.ownerProcessId!=owner||e.type!=expected)return nullptr;if((e.rights&rights)!=rights)return nullptr;return &e;}
bool Close(Table&t,u64 owner,u64 token){if(!owner||!token)return false;usize slot=(usize)((token&0xffffull)-1);if(slot>=MaxHandles)return false;auto&e=t.entries[slot];if(!e.active||e.token!=token||e.ownerProcessId!=owner||(e.rights&RightClose)==0)return false;e={};return true;}
void RevokeProcess(Table&t,u64 owner){for(auto&e:t.entries)if(e.active&&e.ownerProcessId==owner)e={};}
}
