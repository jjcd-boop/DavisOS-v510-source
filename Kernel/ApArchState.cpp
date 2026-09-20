#include "ApArchState.hpp"
namespace Davis::ApArchState {
static bool canonical(u64 x){u64 hi=x>>48;return hi==0||hi==0xffff;}
bool Prepare(State&s,u32 apic,u32 logical,u64 cr3,u64 gdt,u64 tss,u64 stack,u64 gs,u64 entry){s={};if(!cr3||!gdt||!tss||!stack||!gs||!entry||(cr3&0xfffull)||(gdt&7)||(tss&7)||(stack&0xfull)||!canonical(stack)||!canonical(gs)||!canonical(entry))return false;s.apicId=apic;s.logicalId=logical;s.cr3=cr3;s.gdt=gdt;s.tss=tss;s.stackTop=stack;s.gsBase=gs;s.entryPoint=entry;s.stage=Stage::Reset;return Validate(s);}
static bool step(State&s,Stage from,Stage to){if(s.stage!=from)return false;s.stage=to;s.transitions++;return Validate(s);}
bool EnterRealMode(State&s){return step(s,Stage::Reset,Stage::RealMode);} bool EnterProtectedMode(State&s){return step(s,Stage::RealMode,Stage::ProtectedMode);} bool PreparePaging(State&s){return step(s,Stage::ProtectedMode,Stage::PagingPrepared);} bool EnterLongMode(State&s){return step(s,Stage::PagingPrepared,Stage::LongMode);} bool InstallPerCpu(State&s){return step(s,Stage::LongMode,Stage::PerCpuReady);} bool MarkOnline(State&s){return step(s,Stage::PerCpuReady,Stage::Online);} bool Fail(State&s){if(s.stage==Stage::Online||s.stage==Stage::Failed)return false;s.stage=Stage::Failed;s.transitions++;return true;}
bool Validate(const State&s){if(!s.cr3||!s.gdt||!s.tss||!s.stackTop||!s.gsBase||!s.entryPoint||(s.cr3&0xfffull)||(s.gdt&7)||(s.tss&7)||(s.stackTop&0xfull)||!canonical(s.stackTop)||!canonical(s.gsBase)||!canonical(s.entryPoint))return false;return (u8)s.stage<=(u8)Stage::Failed;}
}
