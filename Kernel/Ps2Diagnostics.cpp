#include "Ps2Diagnostics.hpp"
#include "../Arch/x86_64/Io.hpp"
namespace Davis::Ps2Diagnostics {
State state{};
static bool waitIn(u32 n=150000){while(n--){if(!(Io::In8(0x64)&2))return true;__asm__ __volatile__("pause");}return false;}
static bool waitOut(u32 n=150000){while(n--){if(Io::In8(0x64)&1)return true;__asm__ __volatile__("pause");}return false;}
static bool cmd(u8 c){if(!waitIn())return false;Io::Out8(0x64,c);return true;}
static bool data(u8 v){if(!waitIn())return false;Io::Out8(0x60,v);return true;}
static bool recv(u8&v){if(!waitOut())return false;v=Io::In8(0x60);state.lastResponse=v;return true;}
static void flush(){for(u32 i=0;i<64;i++){u8 s=Io::In8(0x64);if(!(s&1))break;(void)Io::In8(0x60);}}
static bool kbd(u8 c,u8&resp){if(!data(c)||!recv(resp))return false;return true;}
static bool aux(u8 c,u8&resp){if(!cmd(0xD4)||!data(c)||!recv(resp))return false;return true;}
void Init(){state={};state.probed=true;state.initialStatus=Io::In8(0x64);flush();
 if(cmd(0x20)){u8 c=0;if(recv(c)){state.configByte=c;state.controllerPresent=true;}}
 if(!state.controllerPresent)return;cmd(0xAD);cmd(0xA7);flush();
 if(cmd(0xAB)){u8 t=0xff;if(recv(t)){state.port1Tested=true;state.keyboardTest=t;state.port1Ok=(t==0);}}
 // Poll both ports. Translation stays enabled for the internal keyboard; IRQs stay disabled.
 u8 cfg=state.configByte;cfg&=~0x03u;cfg&=~0x30u;cfg|=0x40u;if(cmd(0x60)&&data(cfg))state.configByte=cfg;
 if(cmd(0xAE))state.keyboardPortEnabled=true;
 u8 r=0;if(kbd(0xFF,r)){state.keyboardAck=(r==0xFA);if(state.keyboardAck){u8 bat=0;if(recv(bat))state.keyboardBat=(bat==0xAA);}}
 r=0;if(kbd(0xF4,r)&&r==0xFA){state.keyboardAck=true;state.scanningEnabled=true;}
 // Fully initialize the auxiliary PS/2 pointing-device path. This covers laptops whose
 // touchpad/trackpad is EC-translated to i8042 as well as genuine PS/2 mice.
 if(cmd(0xA8))state.auxPortEnabled=true;
 r=0;if(aux(0xFF,r)&&r==0xFA){state.auxAck=true;u8 bat=0;if(recv(bat)){state.auxReset=true;state.auxBat=(bat==0xAA);if(state.auxBat&&waitOut())(void)Io::In8(0x60);}}
 r=0;if(aux(0xF6,r)&&r==0xFA)state.auxAck=true; // defaults: 3-byte relative packet
 r=0;if(aux(0xF4,r)&&r==0xFA){state.auxAck=true;state.auxScanning=true;}
}
void NoteByte(u8 st,u8 v){state.liveStatus=st;state.obfSeen++;if(st&0x20){state.auxBytes++;state.lastAux=v;}else{state.keyboardBytes++;state.lastScan=v;}}
}
