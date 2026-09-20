#pragma once
#include "../Base/Types.hpp"
namespace Davis::TrapReturn { extern "C" void DavisPrepareTrapReturn(u64 kernelRsp,u64 kernelCr3,void(*continuation)()); }
