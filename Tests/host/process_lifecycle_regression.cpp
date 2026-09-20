#include <cstdio>
#include <cstring>
#include "Kernel/Process.hpp"
#include "Kernel/Dxe.hpp"
using namespace Davis;
static int fail(const char* m){std::fprintf(stderr,"FAIL %s\n",m);return 1;}
int main(){
 alignas(16) unsigned char image[sizeof(Dxe::Header)+16]{};
 auto*h=reinterpret_cast<Dxe::Header*>(image);
 h->magic=Dxe::Magic; h->headerBytes=sizeof(Dxe::Header); h->abiMajor=Dxe::AbiMajor; h->abiMinor=Dxe::AbiMinor;
 h->flags=Dxe::PositionIndependent|Dxe::UserMode; h->imageBytes=sizeof(image);
 h->codeOffset=sizeof(Dxe::Header); h->codeBytes=16; h->entryOffset=h->codeOffset; h->requiredStackBytes=65536;
 Process::Table t{}; Process::Init(t);
 // Malformed executables must never consume a process slot or PID.
 auto saved=h->magic; h->magic=0;
 if(Process::CreateValidated(t,image,sizeof(image),0x400000,0x800000))return fail("accepted malformed DXE");
 if(t.nextId!=1)return fail("malformed DXE consumed PID"); h->magic=saved;
 // Repeated application exit/relaunch must not exhaust the fixed process table.
 u64 last=0; constexpr int cycles=4096;
 for(int i=0;i<cycles;i++){
  auto*p=Process::CreateValidated(t,image,sizeof(image),0x400000,0x800000);
  if(!p)return fail("process table exhausted after normal exits");
  if(p->id<=last)return fail("PID not monotonic"); last=p->id;
  if(p->state!=Process::State::Created||p->domain!=SecurityDomain::Kind::Application)return fail("bad initial process state/domain");
  Process::MarkExited(*p,i&255);
  if(p->state!=Process::State::Exited||p->exitCode!=(i&255))return fail("exit state/code not preserved");
 }
 // Fill all live slots: the next launch must fail closed rather than overwrite a live process.
 Process::Init(t);
 Process::Image* live[Process::MaxProcesses]{};
 for(usize i=0;i<Process::MaxProcesses;i++){live[i]=Process::CreateValidated(t,image,sizeof(image),0x400000+i*0x10000,0x800000+i*0x10000);if(!live[i])return fail("premature table exhaustion");}
 if(Process::CreateValidated(t,image,sizeof(image),0x900000,0xA00000))return fail("overwrote live process slot");
 // Once one process exits, exactly that reusable capacity becomes available again with a fresh PID.
 const u64 old=live[17]->id; Process::MarkExited(*live[17],7);
 auto*r=Process::CreateValidated(t,image,sizeof(image),0xB00000,0xC00000);
 if(!r||r->id<=old||r->state!=Process::State::Created)return fail("failed to recycle terminated slot safely");
 std::printf("PASS process_lifecycle cycles=%d max_live=%zu recycled_pid=%llu\n",cycles,(size_t)Process::MaxProcesses,(unsigned long long)r->id);
 return 0;
}
