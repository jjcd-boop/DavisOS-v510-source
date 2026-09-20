#pragma once
#include "Process.hpp"
namespace Davis::RuntimeCurrent {
struct State { Process::Image* process; u64 updates; };
extern State state;
void Set(Process::Image*);
Process::Image* Get();
}
