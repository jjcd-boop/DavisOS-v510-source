#include "ContextSwitch.hpp"
#include "CommitGuard.hpp"
#include "StackRuntime.hpp"
namespace Davis::ContextSwitch {
static State* g=nullptr;
static inline u64 ReadCr3(){u64 v;__asm__ __volatile__("mov %%cr3,%0":"=r"(v));return v;}
static inline void WriteCr3(u64 v){__asm__ __volatile__("mov %0,%%cr3"::"r"(v):"memory");}
void Init(State&s,UserMode::GdtState&gdt){s={};s.gdt=&gdt;}
void SetGlobal(State*s){g=s;}
bool StageGlobal(CpuContext::Frame*f,u64 cr3,u64 rsp0,bool shared){return g&&g->hardwareEnabled&&Stage(*g,f,cr3,rsp0,shared);}
bool StageGlobalCompletion(CpuContext::Frame*f,u64 cr3,u64 rsp0,bool shared,CommitCallback cb,void*cookie){return g&&g->hardwareEnabled&&Stage(*g,f,cr3,rsp0,shared,cb,cookie);}
void Cancel(State&s){s.pending={};}
bool Stage(State&s,CpuContext::Frame*f,u64 cr3,u64 rsp0,bool shared,CommitCallback cb,void*cookie){
 if(!f||!CpuContext::CanonicalRip(*f)||!cr3||(cr3&0xfffull)||!rsp0||(rsp0&0xfull)||!shared){s.rejects++;return false;}
 s.pending={f,cr3,rsp0,shared,true,cb,cookie};return true;
}
extern "C" CpuContext::Frame* DavisCommitContextSwitch(CpuContext::Frame*fallback){
 if(!g||!g->hardwareEnabled||!g->gdt||!g->pending.valid)return fallback;
 Target t=g->pending;g->pending={};if(!t.sharedKernelMappings){g->rejects++;return fallback;}
 if(!StackRuntime::ValidateAnyTarget(t.kernelRsp0,StackRuntime::EntryKind::ContextCommit)){g->hardwareEnabled=false;g->rejects++;return fallback;}
 if(!CommitGuard::Begin(t.cr3,t.kernelRsp0)){g->hardwareEnabled=false;g->rejects++;return fallback;}
 g->gdt->tss.rsp0=t.kernelRsp0;
 CommitGuard::BeforeCr3();
 if((ReadCr3()&~0xfffull)!=(t.cr3&~0xfffull))WriteCr3(t.cr3);
 CommitGuard::Completing();
 g->commits++;if(t.onCommit)t.onCommit(t.frame,t.cr3,t.kernelRsp0,t.cookie);
 CommitGuard::Complete();return t.frame;
}
}
