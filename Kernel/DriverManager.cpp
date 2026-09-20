#include "DriverManager.hpp"
#include "Dxe2.hpp"
namespace Davis::DriverManager {
Table table{};
void Init(){table={};table.ready=true;}
bool AssignTrusted(u64 pid,u64 dev){if(!table.ready||!pid||!dev)return false;for(auto&a:table.assignments)if(a.active&&(a.processId==pid||a.deviceId==dev))return a.processId==pid&&a.deviceId==dev;for(auto&a:table.assignments)if(!a.active){a={pid,dev,true};return true;}return false;}
u64 AssignedDevice(u64 pid){for(auto&a:table.assignments)if(a.active&&a.processId==pid)return a.deviceId;return 0;}
const Driver* FindByDevice(u64 dev){for(auto&d:table.drivers)if(d.active&&d.deviceId==dev)return &d;return nullptr;}
bool Register(Process::Image&p,u64 dev){u64 assigned=AssignedDevice(p.id);if(!assigned||dev!=assigned)return false;if(!table.ready||!dev||p.domain!=SecurityDomain::Kind::Driver||!p.dxe2||(p.grantedCapabilities&Dxe2::CapDriverIo)==0||FindByDevice(dev))return false;for(auto&d:table.drivers)if(!d.active&&d.deviceId==dev){if(d.failures>=3){d.state=State::Quarantined;return false;}u32 failures=d.failures;d={p.id,dev,State::Registered,0,failures,true};return true;}for(auto&d:table.drivers)if(!d.active&&!d.deviceId){d={p.id,dev,State::Registered,0,0,true};return true;}return false;}
bool Heartbeat(const Process::Image&p,u64 dev){for(auto&d:table.drivers)if(d.active&&d.processId==p.id&&d.deviceId==dev){d.heartbeat++;d.state=State::Running;return true;}return false;}
void MarkFailed(u64 pid){for(auto&d:table.drivers)if(d.active&&d.processId==pid){d.failures++;d.state=d.failures>=3?State::Quarantined:State::Failed;}}
void RevokeProcess(u64 pid){for(auto&d:table.drivers)if(d.active&&d.processId==pid){d.active=false;d.processId=0;if(d.state!=State::Quarantined)d.state=d.failures?State::Failed:State::Empty;}}
}
