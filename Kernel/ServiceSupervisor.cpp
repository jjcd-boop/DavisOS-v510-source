#include "ServiceSupervisor.hpp"
namespace Davis::ServiceSupervisor {
State state{};static Slot* at(Service s){u32 i=(u32)s;return i<(u32)Service::Count?&state.slot[i]:nullptr;}
void Init(){state={};for(u32 i=0;i<(u32)Service::Count;i++){auto&x=state.slot[i];x.service=(Service)i;x.health=Health::Offline;x.isolated=true;x.critical=(i==(u32)Service::Desktop||i==(u32)Service::Input);}state.ready=true;}
void MarkStarting(Service s,u64 d){if(auto*x=at(s)){x->health=Health::Starting;x->lastProgress=state.epoch;x->deadlineEpoch=state.epoch+(d?d:1);}}
void MarkRunning(Service s){if(auto*x=at(s)){x->health=Health::Running;x->lastProgress=state.epoch;x->deadlineEpoch=state.epoch+512;}}
void Beat(Service s){if(auto*x=at(s)){x->heartbeat++;x->lastProgress=state.epoch;x->deadlineEpoch=state.epoch+512;if(x->health==Health::Starting||x->health==Health::Degraded)x->health=Health::Running;}}
void MarkFailed(Service s){if(auto*x=at(s)){x->failures++;if(x->failures>=3){x->health=Health::Quarantined;x->restartAtEpoch=0;}else{x->health=Health::RestartBackoff;u64 backoff=128ull<<(x->failures-1);x->restartAtEpoch=state.epoch+backoff;}}}
void Tick(){state.epoch++;for(auto&x:state.slot){if((x.health==Health::Running||x.health==Health::Starting||x.health==Health::Degraded)&&x.deadlineEpoch&&state.epoch>x.deadlineEpoch){if(x.critical&&x.health==Health::Running){x.health=Health::Degraded;x.deadlineEpoch=state.epoch+128;}else MarkFailed(x.service);}}}
bool RestartDue(Service s){auto*x=at(s);return x&&x->health==Health::RestartBackoff&&x->restartAtEpoch&&state.epoch>=x->restartAtEpoch;}
void NoteRestarted(Service s){if(auto*x=at(s)){x->restarts++;x->health=Health::Starting;x->lastProgress=state.epoch;x->deadlineEpoch=state.epoch+256;}}
const Slot* Get(Service s){return at(s);}
}
