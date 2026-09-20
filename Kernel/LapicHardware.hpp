#pragma once
#include "LapicIpi.hpp"
namespace Davis::LapicHardware {
constexpr u32 IcrLowOffset=0x300;
constexpr u32 IcrHighOffset=0x310;
constexpr u32 X2ApicIcrMsr=0x830;
constexpr u32 DeliveryStatusBit=1u<<12;
constexpr u32 DefaultDeliveryPollLimit=100000;
struct Access {
 bool (*readMmio32)(void*,u64,u32&);
 bool (*writeMmio32)(void*,u64,u32);
 bool (*readMsr64)(void*,u32,u64&);
 bool (*writeMsr64)(void*,u32,u64);
 void (*delayUs)(void*,u32);
 void* context;
 u32 pollLimit;
};
struct State { LapicIpi::Mode mode; u64 lapicBase; Access access; u64 writes; u64 polls; bool ready; };
bool Init(State&,LapicIpi::Mode,u64,const Access&);
bool WriteIcr(void*,u64);
void DelayUs(void*,u32);
LapicIpi::Transport MakeTransport(State&);
}
