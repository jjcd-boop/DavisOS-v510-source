#pragma once
#include "../Base/Types.hpp"
#include "Process.hpp"
#include "Dma.hpp"
namespace Davis::DriverGateway {
static constexpr u64 UserMmioBase=0x0000000060000000ull;
static constexpr u64 UserMmioLimit=0x0000000070000000ull;
static constexpr u32 MaxMappings=64,MaxIrqBindings=64,MaxDmaMappings=64;
struct Mapping{u64 processId,resourceId,userBase,pages,addressSpaceCr3;bool active;};
struct DmaMapping{u64 processId,userBase,pages,addressSpaceCr3;Memory::DmaBuffer buffer;bool active;};
struct IrqBinding{u64 processId,resourceId,endpointId;u64 delivered,dropped;bool active;};
struct State{Mapping mappings[MaxMappings];DmaMapping dma[MaxDmaMappings];IrqBinding irqs[MaxIrqBindings];bool ready;};
extern State state;
void Init();
bool MapMmio(const Process::Image&,u64 resourceId,u64*userAddress);
bool PortRead(const Process::Image&,u64 resourceId,u64 offset,u32 width,u32*value);
bool PortWrite(const Process::Image&,u64 resourceId,u64 offset,u32 width,u32 value);
bool AllocateDma(const Process::Image&,u64 bytes,u64 alignment,u64*userAddress,u64*physicalAddress);
bool FreeDma(const Process::Image&,u64 userAddress);
bool BindIrq(const Process::Image&,u64 resourceId,u64 endpointId);
void DeliverIrq(u64 irqNumber);
void RevokeProcess(u64 processId);
}
