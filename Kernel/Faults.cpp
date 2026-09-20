#include "Faults.hpp"
namespace Davis::Faults {
bool IsUserCodeSelector(u64 cs){return (cs&3u)==3u;}
void Contain(Process::Image*p,const Report&r){if(p&&r.fromUser)p->state=Process::State::Faulted;}
}
