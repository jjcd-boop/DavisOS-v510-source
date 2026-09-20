#include "PreemptionProof.hpp"
namespace Davis::PreemptionProof {
void Fail(State&s,Fault f){if(s.fault==Fault::None)s.fault=f;s.armed=false;s.passed=false;}
static bool aligned(u64 x){return x && !(x&0xfffull);}
bool Arm(State&s,const Endpoint&a,const Endpoint&b){s={};s.a=a;s.b=b;
 if(!a.processId||!b.processId){Fail(s,Fault::MissingProcess);return false;}
 if(a.processId==b.processId){Fail(s,Fault::SameProcess);return false;}
 if(!aligned(a.cr3)||!aligned(b.cr3)){Fail(s,Fault::UnalignedCr3);return false;}
 if(a.cr3==b.cr3){Fail(s,Fault::SameCr3);return false;}
 if(!aligned(a.kernelRsp0)||!aligned(b.kernelRsp0)){Fail(s,Fault::UnalignedRsp0);return false;}
 if(a.kernelRsp0==b.kernelRsp0){Fail(s,Fault::SameRsp0);return false;}
 if(!a.sharedKernelMappings||!b.sharedKernelMappings){Fail(s,Fault::MissingKernelContract);return false;}
 s.armed=true;return true;
}
static const Endpoint* endpoint(const State&s,u64 id){return id==s.a.processId?&s.a:id==s.b.processId?&s.b:nullptr;}
bool Observe(State&s,u64 id,u64 cr3,u64 rsp0){if(!s.armed)return false;auto*e=endpoint(s,id);if(!e){Fail(s,Fault::MissingProcess);return false;}if(e->cr3!=cr3||e->kernelRsp0!=rsp0){Fail(s,Fault::WrongSequence);return false;}if(s.lastProcessId&&s.lastProcessId!=id)s.observedSwitches++;s.lastProcessId=id;return true;}
bool ObserveFaultIsolation(State&s,u64 faulted,u64 survivor){if(!s.armed||faulted==survivor||!endpoint(s,faulted)||!endpoint(s,survivor)){Fail(s,Fault::FaultDidNotIsolate);return false;}s.faultedProcessId=faulted;if(s.lastProcessId!=survivor){Fail(s,Fault::FaultDidNotIsolate);return false;}s.passed=s.observedSwitches>=2;return s.passed;}
}
