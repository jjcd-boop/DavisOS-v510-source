#include "StackRuntime.hpp"
namespace Davis::StackRuntime {
State state{};
void Init(){state={};}
static Record* Find(u64 id){for(auto&r:state.record)if(r.valid&&r.processId==id)return &r;return nullptr;}
bool Register(u64 id,const StackGuard::Stack&s){if(!id||!s.ready||!s.usableBase||s.top<=s.usableBase||!s.canary)return false;auto*r=Find(id);if(!r)for(auto&x:state.record)if(!x.valid){r=&x;break;}if(!r)return false;*r={id,s.usableBase,s.top,s.canary,true};return true;}
static bool Fail(EntryKind k,u64 id,u64 rsp,bool canary){state.failures++;if(canary)state.canaryFailures++;else state.rangeFailures++;state.lastKind=k;state.lastProcessId=id;state.lastRsp=rsp;state.fatal=true;return false;}
bool Validate(u64 id,u64 rsp,EntryKind k){state.checks++;auto*r=Find(id);if(!r||rsp<r->low+sizeof(u64)||rsp>=r->high)return Fail(k,id,rsp,false);if(!r->canary||*r->canary!=StackGuard::Canary)return Fail(k,id,rsp,true);return true;}
bool ValidateTarget(u64 id,u64 rsp0,EntryKind k){state.checks++;auto*r=Find(id);if(!r||rsp0!=r->high)return Fail(k,id,rsp0,false);if(!r->canary||*r->canary!=StackGuard::Canary)return Fail(k,id,rsp0,true);return true;}
bool ValidateAnyTarget(u64 rsp0,EntryKind k){for(auto&r:state.record)if(r.valid&&r.high==rsp0)return ValidateTarget(r.processId,rsp0,k);state.checks++;return Fail(k,0,rsp0,false);}
}
