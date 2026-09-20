#include "UsbCore.hpp"
namespace Davis::Usb {
void Init(Registry&r){r={};r.generation=1;}
Device* FindPort(Registry&r,u8 port){for(u32 i=0;i<MaxDevices;i++)if(r.devices[i].state!=DeviceState::Empty&&r.devices[i].state!=DeviceState::Removed&&r.devices[i].port==port)return &r.devices[i];return nullptr;}
Device* Attach(Registry&r,u8 port,Speed speed){if(!port)return nullptr;if(auto*d=FindPort(r,port))return d;for(u32 i=0;i<MaxDevices;i++){auto&d=r.devices[i];if(d.state==DeviceState::Empty||d.state==DeviceState::Removed){d={};d.port=port;d.speed=speed;d.state=DeviceState::Attached;d.generation=++r.generation;r.attaches++;return &d;}}return nullptr;}
void RemovePort(Registry&r,u8 port){if(auto*d=FindPort(r,port)){d->state=DeviceState::Removed;d->generation=++r.generation;r.removes++;}}
bool ParseDeviceDescriptor(Device&d,const u8*p,usize n){if(!p||n<18||p[0]<18||p[1]!=1)return false;d.bcdUsb=(u16)p[2]|((u16)p[3]<<8);d.vendorId=(u16)p[8]|((u16)p[9]<<8);d.productId=(u16)p[10]|((u16)p[11]<<8);return true;}
bool ParseConfiguration(Device&d,const u8*p,usize n){if(!p||n<9||p[1]!=2)return false;u16 total=(u16)p[2]|((u16)p[3]<<8);if(total<9||total>n)return false;d.interfaceCount=0;Interface*cur=nullptr;usize o=0;while(o<total){u8 len=p[o];if(len<2||o+len>total)return false;u8 type=p[o+1];if(type==4&&len>=9){if(d.interfaceCount>=MaxInterfaces)return false;cur=&d.interfaces[d.interfaceCount++];*cur={};cur->number=p[o+2];cur->alternate=p[o+3];cur->klass=p[o+5];cur->subclass=p[o+6];cur->protocol=p[o+7];cur->valid=true;}else if(type==5&&len>=7&&cur){if(cur->endpointCount>=MaxEndpoints)return false;auto&e=cur->endpoints[cur->endpointCount++];e={p[o+2],p[o+3],(u16)((u16)p[o+4]|((u16)p[o+5]<<8)),p[o+6],true};}o+=len;}d.configuration=p[5];d.state=DeviceState::Configured;return true;}
}
