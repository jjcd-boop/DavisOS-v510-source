#pragma once
#include "../Base/Types.hpp"
namespace Davis::ApRendezvous {
constexpr u32 MaxCpus=64;
enum class Stage:u32 { Empty=0, EnteredLongMode=1, Rendezvous=2, Online=3, Failed=4 };
struct Slot { volatile u64 token; volatile u64 generation; volatile u32 apicId; volatile u32 logicalId; volatile u32 stage; };
struct Mailbox { Slot slots[MaxCpus]; u32 count; bool ready; };
bool Init(Mailbox&,u32 count);
bool Publish(Mailbox&,u32 logicalId,u32 apicId,u64 token,u64 generation,Stage);
bool Observe(const Mailbox&,u32 logicalId,u32 apicId,u64 token,u64 generation,Stage minimum);
bool Fail(Mailbox&,u32 logicalId,u32 apicId,u64 token,u64 generation);
}
