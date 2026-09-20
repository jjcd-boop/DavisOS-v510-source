#include "SecurityDomain.hpp"
namespace Davis::SecurityDomain {
const char* Name(Kind k){switch(k){case Kind::Kernel:return "kernel";case Kind::SystemService:return "service";case Kind::Driver:return "driver";case Kind::Application:return "application";}return "invalid";}
}
