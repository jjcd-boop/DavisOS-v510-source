#pragma once
#include "../Base/Types.hpp"
#include "Pci.hpp"
namespace Davis::SystemStatus {
struct Clock { u8 hour,minute,second,day,month; u16 year; bool valid; };
struct State { Clock clock; bool alarmEnabled,alarmRinging; u8 alarmHour,alarmMinute; bool batteryAcpiHint; u32 ethernetControllers,wifiControllers; u16 ethernetVendor,ethernetDevice,wifiVendor,wifiDevice; u64 polls; };
extern State state;
void Init(); void Poll(); void SetAlarmMinutesFromNow(u32); void ClearAlarm();
}
