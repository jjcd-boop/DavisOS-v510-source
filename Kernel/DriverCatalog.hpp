#pragma once
#include "../Base/Types.hpp"
namespace Davis::DriverCatalog {
enum class Bus:u8{Pci,Usb,Acpi,Ps2};
enum class Family:u8{Input,Usb,Storage,Audio,Ethernet,Wifi,Graphics,Printer};
struct Match{Family family;Bus bus;u16 vendor;u16 device;u8 baseClass;u8 subClass;u8 progIf;u16 score;const char*backend;};
const Match* Table(u32& count);
const Match* BestPci(u16 vendor,u16 device,u8 baseClass,u8 subClass,u8 progIf,Family family);
}
