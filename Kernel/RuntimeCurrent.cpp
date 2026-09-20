#include "RuntimeCurrent.hpp"
#include "UserTrap.hpp"
#include "Exceptions.hpp"
#include "TrapExit.hpp"
namespace Davis::RuntimeCurrent {
State state{};
void Set(Process::Image*p){state.process=p;state.updates++;UserTrap::SetCurrent(p);Exceptions::SetCurrent(p);TrapExit::Arm(p);}
Process::Image* Get(){return state.process;}
}
