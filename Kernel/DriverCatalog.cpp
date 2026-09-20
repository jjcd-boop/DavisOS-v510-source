#include "DriverCatalog.hpp"
namespace Davis::DriverCatalog {
static const Match k[]={
 {Family::Input,Bus::Ps2,0,0,0,0,0,900,"ps2-input"},{Family::Input,Bus::Usb,0,0,3,1,1,950,"usb-hid-keyboard"},{Family::Input,Bus::Usb,0,0,3,1,2,950,"usb-hid-mouse"},{Family::Input,Bus::Acpi,0,0,0,0,0,925,"i2c-hid"},
 {Family::Usb,Bus::Pci,0,0,0x0c,0x03,0x30,1000,"xhci"},{Family::Usb,Bus::Pci,0,0,0x0c,0x03,0x20,700,"ehci"},
 {Family::Storage,Bus::Pci,0,0,0x01,0x08,0x02,1000,"nvme"},{Family::Storage,Bus::Pci,0,0,0x01,0x06,0x01,900,"ahci"},{Family::Storage,Bus::Usb,0,0,8,6,0x50,850,"usb-mass-storage"},
 {Family::Audio,Bus::Pci,0x8086,0,0x04,0x03,0,980,"intel-hda"},{Family::Audio,Bus::Pci,0x1022,0,0x04,0x03,0,970,"amd-hda"},{Family::Audio,Bus::Pci,0,0,0x04,0x03,0,800,"hda-generic"},{Family::Audio,Bus::Usb,0,0,1,1,0,850,"usb-audio"},
 {Family::Ethernet,Bus::Pci,0x8086,0,0x02,0x00,0,950,"intel-e1000-family"},{Family::Ethernet,Bus::Pci,0x10ec,0,0x02,0x00,0,940,"realtek-rtl81xx"},{Family::Ethernet,Bus::Pci,0,0,0x02,0x00,0,500,"ethernet-generic"},
 {Family::Wifi,Bus::Pci,0x8086,0x9df0,0x02,0x80,0,995,"intel-wireless-ac-9560"},{Family::Wifi,Bus::Pci,0x8086,0x2723,0x02,0x80,0,995,"intel-wifi6-ax200"},{Family::Wifi,Bus::Pci,0x8086,0x06f0,0x02,0x80,0,995,"intel-wifi6-ax201"},{Family::Wifi,Bus::Pci,0x8086,0,0x02,0x80,0,960,"intel-wifi-family"},{Family::Wifi,Bus::Pci,0x10ec,0,0x02,0x80,0,950,"realtek-wifi-family"},{Family::Wifi,Bus::Pci,0x168c,0,0x02,0x80,0,950,"qualcomm-atheros-family"},{Family::Wifi,Bus::Pci,0x14c3,0x7961,0x02,0x80,0,995,"mediatek-mt7921"},{Family::Wifi,Bus::Pci,0x14c3,0,0x02,0x80,0,950,"mediatek-wifi-family"},{Family::Wifi,Bus::Pci,0,0,0x02,0x80,0,400,"wifi-unsupported-fallback"},
 {Family::Graphics,Bus::Pci,0x8086,0,0x03,0,0,900,"intel-gpu-family"},{Family::Graphics,Bus::Pci,0x1002,0,0x03,0,0,900,"amd-gpu-family"},{Family::Graphics,Bus::Pci,0x10de,0,0x03,0,0,900,"nvidia-gpu-family"},{Family::Graphics,Bus::Pci,0,0,0x03,0,0,500,"framebuffer-fallback"}
};
const Match* Table(u32&count){count=(u32)(sizeof(k)/sizeof(k[0]));return k;}
const Match* BestPci(u16 v,u16 d,u8 bc,u8 sc,u8 pi,Family f){const Match*best=nullptr;u16 bs=0;for(const auto&m:k){if(m.bus!=Bus::Pci||m.family!=f)continue;if(m.vendor&&m.vendor!=v)continue;if(m.device&&m.device!=d)continue;if(m.baseClass&&m.baseClass!=bc)continue;if(m.subClass&&m.subClass!=sc)continue;if(m.progIf&&m.progIf!=pi)continue;if(m.score>bs){best=&m;bs=m.score;}}return best;}
}
