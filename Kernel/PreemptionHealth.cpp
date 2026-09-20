#include "PreemptionHealth.hpp"
namespace Davis::PreemptionHealth {
void Init(State&s,u32 hz,u32 grace){s={};s.status=Status::Cold;s.expectedHz=hz;s.graceMilliseconds=grace?grace:100;}
void Arm(State&s,u64 now,u64 ints){if(!s.expectedHz){s.status=Status::Failed;return;}s.armedTsc=now;s.lastCheckTsc=now;s.lastInterrupts=ints;s.observedInterrupts=0;s.activationCommitted=true;s.status=Status::Armed;}
bool Observe(State&s,u64 now,u64 hz,u64 ints){if(!s.activationCommitted||!hz||now<s.armedTsc||ints<s.lastInterrupts){s.status=Status::Failed;return false;}s.observedInterrupts=ints;s.lastCheckTsc=now;u64 elapsed=now-s.armedTsc;u64 grace=(hz*(u64)s.graceMilliseconds)/1000ull;if(ints>s.lastInterrupts){s.lastInterrupts=ints;s.status=Status::Healthy;return true;}if(elapsed>grace){s.status=Status::Stalled;return false;}return true;}
bool Healthy(const State&s){return s.status==Status::Healthy;}
void Fail(State&s){s.status=Status::Failed;s.activationCommitted=false;}
}
