#include "LapicHardware.hpp"
namespace Davis::LapicHardware {
static bool idle(State&s){
 u32 n=s.access.pollLimit?s.access.pollLimit:DefaultDeliveryPollLimit;
 for(u32 i=0;i<n;i++){
  s.polls++;
  if(s.mode==LapicIpi::Mode::XApic){u32 v=0;if(!s.access.readMmio32(s.access.context,s.lapicBase+IcrLowOffset,v))return false;if(!(v&DeliveryStatusBit))return true;}
  else {u64 v=0;if(!s.access.readMsr64(s.access.context,X2ApicIcrMsr,v))return false;if(!((u32)v&DeliveryStatusBit))return true;}
 }
 return false;
}
bool Init(State&s,LapicIpi::Mode mode,u64 base,const Access&a){s={};if(!a.delayUs||!a.pollLimit)return false;if(mode==LapicIpi::Mode::XApic){if(!base||(base&0xfffull)||!a.readMmio32||!a.writeMmio32)return false;}else if(!a.readMsr64||!a.writeMsr64)return false;s.mode=mode;s.lapicBase=base;s.access=a;s.ready=true;return true;}
bool WriteIcr(void*v,u64 icr){auto&s=*(State*)v;if(!s.ready||!idle(s))return false;if(s.mode==LapicIpi::Mode::XApic){if(!s.access.writeMmio32(s.access.context,s.lapicBase+IcrHighOffset,(u32)(icr>>32)))return false;if(!s.access.writeMmio32(s.access.context,s.lapicBase+IcrLowOffset,(u32)icr))return false;}else if(!s.access.writeMsr64(s.access.context,X2ApicIcrMsr,icr))return false;s.writes++;return idle(s);}
void DelayUs(void*v,u32 us){auto&s=*(State*)v;if(s.ready&&s.access.delayUs)s.access.delayUs(s.access.context,us);}
LapicIpi::Transport MakeTransport(State&s){return {s.mode,s.lapicBase,WriteIcr,DelayUs,&s};}
}
