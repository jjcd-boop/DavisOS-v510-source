#pragma once
#include "../Base/Types.hpp"
namespace Davis::Idt {
struct Gate { u16 off0,selector; u8 ist,typeAttr; u16 off1; u32 off2,reserved; } __attribute__((packed));
struct Table { Gate gate[256]; bool ready; };
void Init(Table&); void SetInterruptGate(Table&,u8 vector,void(*handler)(),u16 selector,u8 dpl,u8 ist=0); bool ValidateSyscallGate(const Table&,u8 vector,u16 kernelCode); void Load(const Table&);
}
