#include "KernelClock.hpp"
namespace Davis::KernelClock {
void Init(State&s,u64 hz){s={};s.frequencyHz=hz;s.calibrated=hz!=0;}
void Tick(State&s){++s.ticks;}
u64 Milliseconds(const State&s){return s.frequencyHz?(s.ticks*1000ull)/s.frequencyHz:0;}
}
