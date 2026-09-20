#include "SystemStatus.hpp"
#include "../Arch/x86_64/Io.hpp"
namespace Davis::SystemStatus { State state{};
static u8 cmos(u8 r){Io::Out8(0x70,(u8)(0x80|r));return Io::In8(0x71);} static u8 bcd(u8 v){return (u8)((v&15)+((v>>4)*10));}
static void readClock(){for(u32 tries=0;tries<8;tries++){if(cmos(0x0A)&0x80)continue;u8 sec=cmos(0),min=cmos(2),hr=cmos(4),day=cmos(7),mon=cmos(8),yr=cmos(9),b=cmos(0x0B);if(cmos(0x0A)&0x80)continue;if(!(b&4)){sec=bcd(sec);min=bcd(min);u8 pm=hr&0x80;hr=bcd(hr&0x7f);if(!(b&2)){if(pm&&hr<12)hr+=12;if(!pm&&hr==12)hr=0;}day=bcd(day);mon=bcd(mon);yr=bcd(yr);}state.clock={hr,min,sec,day,mon,(u16)(2000+yr),true};return;}state.clock.valid=false;}
void Init(){state={};Pci::State p{};Pci::Scan(p);for(u32 i=0;i<p.count;i++){auto&d=p.devices[i];if(d.classCode!=0x02)continue;if(d.subclass==0x00){state.ethernetControllers++;state.ethernetVendor=d.vendor;state.ethernetDevice=d.device;}else if(d.subclass==0x80){state.wifiControllers++;state.wifiVendor=d.vendor;state.wifiDevice=d.device;}}readClock();}
void Poll(){state.polls++;if((state.polls&63)==0)readClock();if(state.alarmEnabled&&state.clock.valid&&state.clock.hour==state.alarmHour&&state.clock.minute==state.alarmMinute)state.alarmRinging=true;}
void SetAlarmMinutesFromNow(u32 m){readClock();if(!state.clock.valid)return;u32 t=(u32)state.clock.hour*60+state.clock.minute+m;t%=1440;state.alarmHour=(u8)(t/60);state.alarmMinute=(u8)(t%60);state.alarmEnabled=true;state.alarmRinging=false;}
void ClearAlarm(){state.alarmEnabled=false;state.alarmRinging=false;}
}
