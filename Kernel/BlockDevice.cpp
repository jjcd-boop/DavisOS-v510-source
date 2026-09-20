#include "BlockDevice.hpp"
namespace Davis::BlockDevice {
State state{};
static u8 ramDisk[RamDiskSectors*SectorBytes]{};
static void copyBytes(u8*d,const u8*s,u64 n){for(u64 i=0;i<n;i++)d[i]=s[i];}
static const Backend* backend(u32 id){for(u32 i=0;i<MaxDevices;i++)if(state.backends[i].present&&state.backends[i].id==id)return &state.backends[i];return nullptr;}
static bool ramSubmit(const Device&,Request&r){r.backendToken=r.id;return true;}
static bool ramPoll(const Device&,Request&r,bool&done,bool&success,i32&error){
 done=true;success=true;error=0;if(r.op==Op::Flush)return true;u64 off=r.lba*SectorBytes,n=(u64)r.sectorCount*SectorBytes;
 if(r.op==Op::Read)copyBytes(r.buffer,ramDisk+off,n);else if(r.op==Op::Write)copyBytes(ramDisk+off,r.buffer,n);else{success=false;error=-5;}return true;
}
void Init(){state={};state.nextRequestId=1;state.ready=true;RegisterBackend(1,ramSubmit,ramPoll);RegisterDevice(1,RamDiskSectors,SectorBytes,true,1);}
const Device* GetDevice(u32 id){for(u32 i=0;i<MaxDevices;i++)if(state.devices[i].present&&state.devices[i].id==id)return &state.devices[i];return nullptr;}
bool RegisterBackend(u32 id,SubmitBackend s,PollBackend p,CancelBackend c){if(!id||!s||!p||backend(id))return false;for(u32 i=0;i<MaxDevices;i++)if(!state.backends[i].present){state.backends[i]={id,s,p,c,true};return true;}return false;}
bool RegisterDevice(u32 id,u64 sectors,u32 bytes,bool writable,u32 backendId){if(!id||!sectors||!bytes||GetDevice(id)||!backend(backendId))return false;for(u32 i=0;i<MaxDevices;i++)if(!state.devices[i].present){state.devices[i]={id,sectors,bytes,writable,true,backendId};return true;}return false;}
u32 CancelDeviceRequests(u32 deviceId,i32 error){u32 n=0;const Device*d=GetDevice(deviceId);const Backend*b=d?backend(d->backendId):nullptr;for(u32 i=0;i<MaxRequests;i++){auto&r=state.requests[i];if(r.status==Status::Free||r.deviceId!=deviceId)continue;if(r.status==Status::Active&&b&&b->cancel)b->cancel(*d,r);if(r.status==Status::Queued||r.status==Status::Active){r.status=Status::Failed;r.error=error;state.failed++;n++;}}return n;}
bool RemoveDevice(u32 id,bool cancelOutstanding){for(u32 i=0;i<MaxDevices;i++)if(state.devices[i].present&&state.devices[i].id==id){if(cancelOutstanding)CancelDeviceRequests(id);for(u32 q=0;q<MaxRequests;q++){auto&r=state.requests[q];if(r.deviceId==id&&(r.status==Status::Queued||r.status==Status::Active))return false;}state.devices[i]={};return true;}return false;}
bool RemoveBackend(u32 id){for(u32 d=0;d<MaxDevices;d++)if(state.devices[d].present&&state.devices[d].backendId==id)return false;for(u32 i=0;i<MaxDevices;i++)if(state.backends[i].present&&state.backends[i].id==id){state.backends[i]={};return true;}return false;}
u64 Submit(u32 deviceId,Op op,u64 lba,u32 count,u8* buffer,u64 timeout){
 const Device*d=GetDevice(deviceId);if(!d)return 0;if(op==Op::Flush){lba=0;count=0;buffer=nullptr;}else if(!count||lba>=d->sectorCount||count>d->sectorCount-lba||!buffer)return 0;if(op==Op::Write&&!d->writable)return 0;
 for(u32 i=0;i<MaxRequests;i++)if(state.requests[i].status==Status::Free){u64 id=state.nextRequestId++;if(!id)id=state.nextRequestId++;state.requests[i]={id,deviceId,op,Status::Queued,lba,count,buffer,state.ticks,state.ticks+(timeout?timeout:1),0,0};return id;}return 0;
}
Status Query(u64 id,i32* error){for(u32 i=0;i<MaxRequests;i++)if(state.requests[i].status!=Status::Free&&state.requests[i].id==id){if(error)*error=state.requests[i].error;return state.requests[i].status;}if(error)*error=-1;return Status::Free;}
bool Reap(u64 id,i32* error){for(u32 i=0;i<MaxRequests;i++){auto&r=state.requests[i];if(r.status!=Status::Free&&r.id==id){if(r.status!=Status::Complete&&r.status!=Status::Failed&&r.status!=Status::TimedOut)return false;if(error)*error=r.error;r={};return true;}}return false;}
void PumpOnce(){if(!state.ready)return;state.ticks++;
 for(u32 i=0;i<MaxRequests;i++){auto&r=state.requests[i];if(r.status==Status::Free||r.status==Status::Complete||r.status==Status::Failed||r.status==Status::TimedOut)continue;
  if(state.ticks>r.deadlineTick){const Device*td=GetDevice(r.deviceId);const Backend*tb=td?backend(td->backendId):nullptr;if(tb&&tb->cancel&&r.status==Status::Active)tb->cancel(*td,r);r.status=Status::TimedOut;r.error=-2;state.timedOut++;continue;}const Device*d=GetDevice(r.deviceId);if(!d){r.status=Status::Failed;r.error=-3;state.failed++;continue;}const Backend*b=backend(d->backendId);if(!b){r.status=Status::Failed;r.error=-4;state.failed++;continue;}
  if(r.status==Status::Queued){if(!b->submit(*d,r)){r.status=Status::Failed;r.error=-6;state.failed++;continue;}r.status=Status::Active;}
  bool done=false,success=false;i32 err=0;if(!b->poll(*d,r,done,success,err)){r.status=Status::Failed;r.error=err?err:-7;state.failed++;continue;}if(done){r.status=success?Status::Complete:Status::Failed;r.error=err;if(success)state.completed++;else state.failed++;}break;
 }
}
}
