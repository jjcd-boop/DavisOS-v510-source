#include "AppCohort.hpp"
#include "Syscall.hpp"
namespace Davis::AppCohort {
static bool genericReady(const MultiAppRuntime::App&a){const bool cohort=(a.kind!=AppPreemptionA&&a.kind!=AppPreemptionB);return cohort&&a.ready&&a.loaded.process&&(a.loaded.process->state==Process::State::Ready||a.loaded.process->state==Process::State::Running);}
void Build(State&s,MultiAppRuntime::State&r){s={};for(usize i=0;i<r.appCount&&s.count<MaxBootApps;i++){auto&a=r.apps[i];if(!genericReady(a))continue;s.memberIds[s.count++]=a.loaded.process->id;if(!s.firstId)s.firstId=a.loaded.process->id;s.runnable++;}s.armed=s.count>0;}
Process::Image* FirstRunnable(State&s,MultiAppRuntime::State&r){if(!s.armed)return nullptr;for(usize n=0;n<s.count;n++)for(usize i=0;i<r.appCount;i++){auto*p=r.apps[i].loaded.process;if(p&&p->id==s.memberIds[n]&&(p->state==Process::State::Ready||p->state==Process::State::Running))return p;}return nullptr;}
void RetireTerminal(State&s,MultiAppRuntime::State&r){usize live=0;for(usize n=0;n<s.count;n++)for(usize i=0;i<r.appCount;i++){auto*p=r.apps[i].loaded.process;if(p&&p->id==s.memberIds[n]&&(p->state==Process::State::Ready||p->state==Process::State::Running||p->state==Process::State::Blocked))live++;}s.runnable=live;}
void AbortSurvivors(State&s,MultiAppRuntime::State&r,int code){for(usize n=0;n<s.count;n++)for(usize i=0;i<r.appCount;i++){auto*p=r.apps[i].loaded.process;if(!p||p->id!=s.memberIds[n])continue;if(p->state==Process::State::Ready||p->state==Process::State::Running||p->state==Process::State::Blocked){Syscall::RevokeProcessObjects(p->id);Process::MarkExited(*p,code);}}s.runnable=0;s.armed=false;}
}
