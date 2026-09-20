#include "Kernel/ServiceSupervisor.hpp"
#include <cstdio>
using namespace Davis;
int main(){using namespace ServiceSupervisor;Init();MarkRunning(Service::Network);for(int i=0;i<514;i++)Tick();auto*n=Get(Service::Network);if(!n||n->health!=Health::RestartBackoff||n->failures!=1)return 1;while(!RestartDue(Service::Network))Tick();NoteRestarted(Service::Network);Beat(Service::Network);if(Get(Service::Network)->health!=Health::Running)return 2;
 MarkRunning(Service::Desktop);for(int i=0;i<514;i++)Tick();if(Get(Service::Desktop)->health!=Health::Degraded)return 3;for(int i=0;i<130;i++)Tick();if(Get(Service::Desktop)->health!=Health::RestartBackoff)return 4;
 while(!RestartDue(Service::Desktop))Tick();NoteRestarted(Service::Desktop);MarkFailed(Service::Desktop);while(!RestartDue(Service::Desktop))Tick();NoteRestarted(Service::Desktop);MarkFailed(Service::Desktop);if(Get(Service::Desktop)->health!=Health::Quarantined)return 5;
 std::printf("PASS service_supervisor timeout degrade restart quarantine\n");return 0;}
