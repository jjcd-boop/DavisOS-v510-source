#include "Process.hpp"
namespace Davis::Process {
void Init(Table&t){t={};t.nextId=1;}
Image* CreateValidated(Table&t,const void*img,u64 bytes,u64 mappedBase,u64 stack){
 const Dxe::Header*h=nullptr;if(Dxe::Validate(img,bytes,&h)!=Dxe::ValidateResult::Ok)return nullptr;
 for(usize i=0;i<MaxProcesses;i++){ if(t.process[i].state==State::Empty||t.process[i].state==State::Exited||t.process[i].state==State::Faulted){
  auto&p=t.process[i];p={};p.id=t.nextId++;p.state=State::Created;p.header=h;
  p.fileImage=(const u8*)img;p.fileBytes=bytes;p.entryVirtual=mappedBase+h->entryOffset;
  p.userStackTop=stack;p.domain=SecurityDomain::Kind::Application;return &p;
 }}
 return nullptr;
}
void MarkExited(Image&p,int code){p.exitCode=code;p.state=State::Exited;}
}
