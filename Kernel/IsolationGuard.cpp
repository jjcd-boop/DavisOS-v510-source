#include "IsolationGuard.hpp"
namespace Davis::IsolationGuard {
State state{};
void Init(){state={};state.ready=true;}
static Cell* slot(u64 pid){for(auto&c:state.cells)if(c.active&&c.processId==pid)return &c;return nullptr;}
const Cell* Find(u64 pid){return slot(pid);}
void Admit(const Process::Image&p){if(!state.ready||!p.id)return;if(auto*c=slot(p.id)){c->domain=p.domain;return;}for(auto&c:state.cells)if(!c.active){c={p.id,p.domain,Status::Healthy,0,0,FaultKind::None,0,true};return;}}
void RecordFault(const Process::Image&p,FaultKind k,u64 detail){Admit(p);auto*c=slot(p.id);if(!c)return;c->faults++;c->lastFault=k;c->lastVector=detail;c->status=c->faults>=3?Status::Quarantined:Status::Degraded;}
bool BeginTeardown(u64 pid){auto*c=slot(pid);if(!c)return true;if(c->status==Status::TearingDown||c->status==Status::Dead)return false;c->status=Status::TearingDown;c->teardownGeneration++;return true;}
void FinishTeardown(u64 pid){if(auto*c=slot(pid))c->status=Status::Dead;}
bool MayUsePrivilegedDriverPath(const Process::Image&p){if(p.domain!=SecurityDomain::Kind::Driver)return false;auto*c=slot(p.id);if(!c)return true;return c->status!=Status::Quarantined&&c->status!=Status::TearingDown&&c->status!=Status::Dead;}
}
