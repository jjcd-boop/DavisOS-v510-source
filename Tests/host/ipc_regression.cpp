#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "../../Kernel/Ipc.hpp"
using namespace Davis;
static void req(bool v,const char*m){if(!v){std::fprintf(stderr,"FAIL: %s\n",m);std::exit(2);}}
int main(){
 Ipc::Init();
 auto ep=Ipc::CreateEndpoint(42,Ipc::RightSend|Ipc::RightReceive); req(ep!=0,"endpoint create");
 const char msg[]="davis-ipc"; req(Ipc::Send(7,ep,99,msg,sizeof(msg)),"send basic");
 char out[32]{};u64 sender=0;u32 type=0;auto n=Ipc::Receive(42,ep,&sender,&type,out,sizeof(out));
 req(n==(i64)sizeof(msg)&&sender==7&&type==99&&!std::memcmp(out,msg,sizeof(msg)),"receive integrity");
 req(Ipc::Receive(42,ep,nullptr,nullptr,out,sizeof(out))==0,"empty queue");
 req(Ipc::Receive(41,ep,nullptr,nullptr,out,sizeof(out))==-1,"owner isolation");
 req(!Ipc::Send(7,ep,1,msg,Ipc::MaxPayload+1),"oversize rejected");
 for(int i=0;i<8;i++)req(Ipc::Send(100+i,ep,(u32)i,msg,sizeof(msg)),"queue fill");
 req(!Ipc::Send(999,ep,9,msg,sizeof(msg)),"queue overflow rejected");
 char tiny[1]{};req(Ipc::Receive(42,ep,nullptr,nullptr,tiny,sizeof(tiny))==-2,"short buffer rejected");
 // Short-buffer failure must not consume the queued message.
 n=Ipc::Receive(42,ep,&sender,&type,out,sizeof(out));req(n==(i64)sizeof(msg)&&sender==100&&type==0,"short buffer preserved message");
 // Churn endpoints and ensure ownership/revocation is deterministic.
 u64 ids[Ipc::MaxEndpoints]{};Ipc::Init();
 for(usize i=0;i<Ipc::MaxEndpoints;i++){ids[i]=Ipc::CreateEndpoint(1000+i,Ipc::RightReceive);req(ids[i]!=0,"endpoint capacity");}
 req(Ipc::CreateEndpoint(9999,Ipc::RightReceive)==0,"endpoint exhaustion rejected");
 Ipc::RevokeProcess(1005);req(Ipc::OwnerOf(ids[5])==0,"revocation");req(Ipc::CreateEndpoint(7777,Ipc::RightReceive)!=0,"slot reuse");
 // Sustained FIFO stress on one endpoint.
 Ipc::Init();ep=Ipc::CreateEndpoint(55,Ipc::RightReceive);req(ep,"stress endpoint");
 for(unsigned round=0;round<2000;round++){
  unsigned value=round;
  req(Ipc::Send(66,ep,round,&value,sizeof(value)),"stress send");
  unsigned got=0; sender=0;type=0;n=Ipc::Receive(55,ep,&sender,&type,&got,sizeof(got));
  req(n==4&&sender==66&&type==round&&got==round,"stress fifo/integrity");
 }
 std::puts("PASS ipc_regression isolation capacity revoke fifo stress=2000");return 0;
}
