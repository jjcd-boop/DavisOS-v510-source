static inline long call(unsigned long n,unsigned long a=0,unsigned long b=0,unsigned long c=0,unsigned long d=0){register unsigned long rax asm("rax")=n;register unsigned long rdi asm("rdi")=a;register unsigned long rsi asm("rsi")=b;register unsigned long rdx asm("rdx")=c;register unsigned long r10 asm("r10")=d;asm volatile("int $0x80":"+a"(rax):"D"(rdi),"S"(rsi),"d"(rdx),"r"(r10):"memory","cc");return (long)rax;}
struct DriverDmaInfo{unsigned long user,physical,bytes;};
struct DriverResourceInfo{unsigned long id,base,length;unsigned int type,rights,reserved;};
struct Trb{unsigned int parameterLo,parameterHi,status,control;};
struct XhciCaps{unsigned char capLength;unsigned short version;unsigned int hcs1,hcs2,hcs3,hcc1,dboff,rtsoff;unsigned int maxSlots,maxPorts;bool valid;};
struct Ring{DriverDmaInfo dma;unsigned int enqueue;unsigned int dequeue;bool cycle;bool ready;};
static volatile unsigned int* reg32(unsigned long base,unsigned long off){return (volatile unsigned int*)(base+off);}
static XhciCaps probe(unsigned long mmio,unsigned long length){XhciCaps c{};if(!mmio||length<0x20)return c;volatile unsigned char* b=(volatile unsigned char*)mmio;c.capLength=b[0];c.version=*(volatile unsigned short*)(mmio+2);c.hcs1=*reg32(mmio,4);c.hcs2=*reg32(mmio,8);c.hcs3=*reg32(mmio,12);c.hcc1=*reg32(mmio,16);c.dboff=*reg32(mmio,20);c.rtsoff=*reg32(mmio,24);c.maxSlots=c.hcs1&0xff;c.maxPorts=(c.hcs1>>24)&0xff;c.valid=c.capLength>=0x20&&c.capLength<0x100&&c.version>=0x0090&&c.version<=0x0200&&c.dboff<length&&c.rtsoff<length&&c.maxSlots;return c;}
static bool allocDma(unsigned long bytes,unsigned long align,DriverDmaInfo&d){d={};return call(74,bytes,align,(unsigned long)&d)==0&&d.user&&d.physical&&d.bytes>=bytes;}
static void zeroDma(DriverDmaInfo&d){volatile unsigned char*p=(volatile unsigned char*)d.user;for(unsigned long i=0;i<d.bytes;i++)p[i]=0;}
static bool initCommandRing(Ring&r){if(!allocDma(4096,64,r.dma))return false;zeroDma(r.dma);Trb*t=(Trb*)r.dma.user;unsigned int last=(unsigned int)(r.dma.bytes/sizeof(Trb))-1;t[last].parameterLo=(unsigned int)r.dma.physical;t[last].parameterHi=(unsigned int)(r.dma.physical>>32);t[last].control=(6u<<10)|(1u<<1)|1u;r.enqueue=0;r.dequeue=0;r.cycle=true;r.ready=true;return true;}
static bool initEventRing(Ring&r,DriverDmaInfo&erst){if(!allocDma(4096,64,r.dma)||!allocDma(4096,64,erst))return false;zeroDma(r.dma);zeroDma(erst);volatile unsigned long*e=(volatile unsigned long*)erst.user;e[0]=r.dma.physical;e[1]=r.dma.bytes/sizeof(Trb);r.enqueue=0;r.dequeue=0;r.cycle=true;r.ready=true;return true;}


static void ringDoorbell(unsigned long mmio,const XhciCaps&c,unsigned int slot,unsigned int target=0){*reg32(mmio+(c.dboff&~3u),slot*4)=target;}
static bool queueCommand(Ring&r,unsigned int type,unsigned long parameter=0,unsigned int status=0,unsigned int extra=0){if(!r.ready)return false;Trb*t=(Trb*)r.dma.user;unsigned int count=(unsigned int)(r.dma.bytes/sizeof(Trb))-1;if(r.enqueue>=count)r.enqueue=0;Trb&q=t[r.enqueue++];q.parameterLo=(unsigned int)parameter;q.parameterHi=(unsigned int)(parameter>>32);q.status=status;q.control=(type<<10)|extra|(r.cycle?1u:0u);return true;}
static bool pollEvent(Ring&r,Trb&out,unsigned long spins=8000000){Trb*t=(Trb*)r.dma.user;unsigned int count=(unsigned int)(r.dma.bytes/sizeof(Trb));while(spins--){volatile Trb*e=&t[r.dequeue];unsigned int ctl=e->control;if((ctl&1u)==(r.cycle?1u:0u)){out=*(const Trb*)e;r.dequeue++;if(r.dequeue>=count){r.dequeue=0;r.cycle=!r.cycle;}return true;}asm volatile("pause");}return false;}
static bool enableFirstSlot(unsigned long mmio,const XhciCaps&c,Ring&cmd,Ring&evt,unsigned int&slot){slot=0;if(!queueCommand(cmd,9))return false;ringDoorbell(mmio,c,0);Trb e{};if(!pollEvent(evt,e))return false;unsigned int type=(e.control>>10)&0x3f,cc=(e.status>>24)&0xff;if(type!=33||cc!=1)return false;slot=(e.control>>24)&0xff;return slot!=0;}


static unsigned int contextBytes(const XhciCaps&c){return (c.hcc1&(1u<<2))?64u:32u;}
static unsigned int portSpeed(unsigned int portsc){return (portsc>>10)&0xfu;}
static unsigned int ep0Packet(unsigned int speed){if(speed==4)return 512; if(speed==3)return 64; if(speed==2)return 8; return 8;}
static bool waitPortEnabled(unsigned long op,unsigned int port,unsigned long spins=4000000){volatile unsigned int*p=reg32(op,0x400+port*0x10);while(spins--){unsigned int v=*p;if((v&1u)&&(v&(1u<<1)))return true;asm volatile("pause");}return false;}
static bool resetPort(unsigned long op,unsigned int port){volatile unsigned int*p=reg32(op,0x400+port*0x10);unsigned int v=*p;if(!(v&1u))return false;if(v&(1u<<1))return true;unsigned int preserve=v&0x0e00c3e0u;*p=preserve|(1u<<4);return waitPortEnabled(op,port);}
static bool initTransferRing(Ring&r){if(!allocDma(4096,64,r.dma))return false;zeroDma(r.dma);Trb*t=(Trb*)r.dma.user;unsigned int last=(unsigned int)(r.dma.bytes/sizeof(Trb))-1;t[last].parameterLo=(unsigned int)r.dma.physical;t[last].parameterHi=(unsigned int)(r.dma.physical>>32);t[last].control=(6u<<10)|(1u<<1)|1u;r.enqueue=0;r.dequeue=0;r.cycle=true;r.ready=true;return true;}
static bool addressDevice(unsigned long mmio,const XhciCaps&c,Ring&cmd,Ring&evt,DriverDmaInfo&dcbaa,unsigned int port,unsigned int slot,DriverDmaInfo&outCtx,DriverDmaInfo&inCtx,Ring&ep0){
 if(!slot||!resetPort(mmio+c.capLength,port))return false;
 unsigned int stride=contextBytes(c),speed=portSpeed(*reg32(mmio+c.capLength,0x400+port*0x10));
 if(!allocDma(stride*32,64,outCtx)||!allocDma(stride*33,64,inCtx)||!initTransferRing(ep0))return false;
 zeroDma(outCtx);zeroDma(inCtx);volatile unsigned long*dc=(volatile unsigned long*)dcbaa.user;dc[slot]=outCtx.physical;
 volatile unsigned int*ic=(volatile unsigned int*)inCtx.user;ic[1]=3u; // Add Slot + EP0 contexts.
 volatile unsigned int*sc=(volatile unsigned int*)(inCtx.user+stride);sc[0]=(speed&0xfu)<<20;sc[1]=(port&0xffu)<<16;
 volatile unsigned int*ec=(volatile unsigned int*)(inCtx.user+stride*2);ec[1]=(3u<<1)|(4u<<3)|(ep0Packet(speed)<<16);ec[2]=(unsigned int)(ep0.dma.physical|1ull);ec[3]=(unsigned int)((ep0.dma.physical|1ull)>>32);ec[4]=8;
 if(!queueCommand(cmd,11,inCtx.physical,0,(slot&0xffu)<<24))return false;ringDoorbell(mmio,c,0);Trb e{};if(!pollEvent(evt,e))return false;return ((e.control>>10)&0x3f)==33&&((e.status>>24)&0xff)==1&&((e.control>>24)&0xff)==slot;
}


static bool queueTransfer(Ring&r,unsigned long parameter,unsigned int status,unsigned int control){
 if(!r.ready)return false;Trb*t=(Trb*)r.dma.user;unsigned int count=(unsigned int)(r.dma.bytes/sizeof(Trb))-1;
 if(r.enqueue>=count)r.enqueue=0;Trb&q=t[r.enqueue++];q.parameterLo=(unsigned int)parameter;q.parameterHi=(unsigned int)(parameter>>32);q.status=status;q.control=control|(r.cycle?1u:0u);return true;
}
static bool waitTransferEvent(Ring&evt,unsigned int slot,unsigned int endpoint,Trb&out,unsigned long spins=10000000){
 while(spins--){Trb e{};if(!pollEvent(evt,e,1)){asm volatile("pause");continue;}unsigned int type=(e.control>>10)&0x3fu;if(type!=32)continue;unsigned int eslot=(e.control>>24)&0xffu,ep=(e.control>>16)&0x1fu;if(eslot!=slot||ep!=endpoint)continue;unsigned int cc=(e.status>>24)&0xffu;if(cc!=1&&cc!=13)return false;out=e;return true;}return false;
}
static bool getDeviceDescriptor(unsigned long mmio,const XhciCaps&c,Ring&evt,Ring&ep0,unsigned int slot,DriverDmaInfo&buf,unsigned int&actual){
 actual=0;if(!slot||!allocDma(64,64,buf))return false;zeroDma(buf);
 // USB SETUP: bmRequestType=IN|standard|device, GET_DESCRIPTOR, DEVICE descriptor, index 0, length 18.
 unsigned long setup=0x0012000001000680ull;
 // Setup Stage TRB: immediate 8-byte setup packet, TRT=IN data stage.
 if(!queueTransfer(ep0,setup,8u,(2u<<10)|(1u<<6)|(3u<<16)))return false;
 // Data Stage TRB: 18-byte IN transfer to driver-owned DMA buffer.
 if(!queueTransfer(ep0,buf.physical,18u,(3u<<10)|(1u<<16)))return false;
 // Status Stage TRB: OUT status, IOC so completion is reported.
 if(!queueTransfer(ep0,0,0,(4u<<10)|(1u<<5)))return false;
 ringDoorbell(mmio,c,slot,1);Trb e{};if(!waitTransferEvent(evt,slot,1,e))return false;
 unsigned int residual=e.status&0x00ffffffu;actual=residual<18u?18u-residual:0u;
 volatile unsigned char*d=(volatile unsigned char*)buf.user;
 return actual>=8&&d[0]>=18&&d[1]==1; // DEVICE descriptor header.
}


struct HidEndpoint{unsigned char configuration,interfaceNumber,endpointAddress,interval,subclass,protocol;unsigned short maxPacket,totalLength;bool found;};
struct HidReport{unsigned char protocol,length,reserved0,reserved1,data[64];};
static bool controlIn(unsigned long mmio,const XhciCaps&c,Ring&evt,Ring&ep0,unsigned int slot,unsigned long setup,unsigned int length,DriverDmaInfo&buf,unsigned int&actual){
 actual=0;if(!length||!allocDma((length+63u)&~63u,64,buf))return false;zeroDma(buf);
 if(!queueTransfer(ep0,setup,8u,(2u<<10)|(1u<<6)|(3u<<16)))return false;
 if(!queueTransfer(ep0,buf.physical,length,(3u<<10)|(1u<<16)))return false;
 if(!queueTransfer(ep0,0,0,(4u<<10)|(1u<<5)))return false;
 ringDoorbell(mmio,c,slot,1);Trb e{};if(!waitTransferEvent(evt,slot,1,e))return false;
 unsigned int residual=e.status&0x00ffffffu;actual=residual<length?length-residual:0u;return actual!=0;
}
static bool controlNoData(unsigned long mmio,const XhciCaps&c,Ring&evt,Ring&ep0,unsigned int slot,unsigned long setup){
 if(!queueTransfer(ep0,setup,8u,(2u<<10)|(1u<<6)))return false;
 if(!queueTransfer(ep0,0,0,(4u<<10)|(1u<<5)|(1u<<16)))return false; // IN status stage
 ringDoorbell(mmio,c,slot,1);Trb e{};return waitTransferEvent(evt,slot,1,e);
}
static bool getConfigurationDescriptor(unsigned long mmio,const XhciCaps&c,Ring&evt,Ring&ep0,unsigned int slot,DriverDmaInfo&buf,unsigned int&actual,HidEndpoint&hid){
 actual=0;hid={};DriverDmaInfo head{};unsigned int got=0;
 // GET_DESCRIPTOR(CONFIGURATION), first 9 bytes to learn wTotalLength.
 unsigned long setup9=0x0009000002000680ull;
 if(!controlIn(mmio,c,evt,ep0,slot,setup9,9,head,got)||got<9)return false;
 volatile unsigned char*h=(volatile unsigned char*)head.user;if(h[1]!=2)return false;unsigned int total=(unsigned int)h[2]|((unsigned int)h[3]<<8);if(total<9||total>2048)return false;
 unsigned long setup=((unsigned long)total<<48)|0x0000000002000680ull;
 if(!controlIn(mmio,c,evt,ep0,slot,setup,total,buf,actual)||actual<9)return false;
 volatile unsigned char*d=(volatile unsigned char*)buf.user;hid.configuration=d[5];hid.totalLength=(unsigned short)total;
 bool hidInterface=false;unsigned int off=0;while(off+2<=actual){unsigned int len=d[off],type=d[off+1];if(len<2||off+len>actual)break;
   if(type==4&&len>=9){hidInterface=(d[off+5]==3);if(hidInterface){hid.interfaceNumber=d[off+2];hid.subclass=d[off+6];hid.protocol=d[off+7];}}
   else if(type==5&&len>=7&&hidInterface){unsigned char addr=d[off+2],attr=d[off+3];if((addr&0x80u)&&(attr&3u)==3u){hid.endpointAddress=addr;hid.maxPacket=(unsigned short)(d[off+4]|((unsigned short)d[off+5]<<8));hid.interval=d[off+6];hid.found=hid.maxPacket!=0;return hid.found;}}
   off+=len;
 }return false;
}
static unsigned int intervalForXhci(unsigned int speed,unsigned char bInterval){if(!bInterval)return 1;if(speed>=3){unsigned int v=bInterval;if(v>16)v=16;return v-1;}unsigned int p=0,v=bInterval;while((1u<<p)<v&&p<7)p++;return p+3;}
static bool configureHidEndpoint(unsigned long mmio,const XhciCaps&c,Ring&cmd,Ring&evt,unsigned int slot,unsigned int port,DriverDmaInfo&inCtx,const HidEndpoint&hid,Ring&intrRing,unsigned int&dci){
 if(!hid.found||!initTransferRing(intrRing))return false;unsigned int ep=hid.endpointAddress&0x0fu;dci=ep*2u+1u;if(!ep||dci>=32)return false;
 unsigned int stride=contextBytes(c),speed=portSpeed(*reg32(mmio+c.capLength,0x400+(port-1)*0x10));zeroDma(inCtx);volatile unsigned int*ic=(volatile unsigned int*)inCtx.user;
 ic[1]=(1u<<0)|(1u<<dci); // Add Slot plus discovered interrupt-IN endpoint.
 volatile unsigned int*sc=(volatile unsigned int*)(inCtx.user+stride);sc[0]=(speed&0xfu)<<20;sc[1]=((port&0xffu)<<16)|(dci<<27);
 volatile unsigned int*ec=(volatile unsigned int*)(inCtx.user+stride*(dci+1));ec[0]=intervalForXhci(speed,hid.interval)<<16;ec[1]=(3u<<1)|(7u<<3)|((unsigned int)hid.maxPacket<<16);ec[2]=(unsigned int)(intrRing.dma.physical|1ull);ec[3]=(unsigned int)((intrRing.dma.physical|1ull)>>32);ec[4]=hid.maxPacket;
 if(!queueCommand(cmd,12,inCtx.physical,0,(slot&0xffu)<<24))return false;ringDoorbell(mmio,c,0);Trb e{};if(!pollEvent(evt,e))return false;return ((e.control>>10)&0x3fu)==33&&((e.status>>24)&0xffu)==1&&((e.control>>24)&0xffu)==slot;
}
static bool setConfiguration(unsigned long mmio,const XhciCaps&c,Ring&evt,Ring&ep0,unsigned int slot,unsigned char config){unsigned long setup=((unsigned long)config<<16)|0x0000000000000900ull;return config&&controlNoData(mmio,c,evt,ep0,slot,setup);}

static bool setBootProtocol(unsigned long mmio,const XhciCaps&c,Ring&evt,Ring&ep0,unsigned int slot,const HidEndpoint&hid){
 if(hid.subclass!=1||(hid.protocol!=1&&hid.protocol!=2))return true; // non-boot HID stays report protocol
 // HID SET_PROTOCOL(boot=0): bmRequestType=0x21, bRequest=0x0b, wValue=0, wIndex=interface.
 unsigned long setup=((unsigned long)hid.interfaceNumber<<32)|0x0000000000000b21ull;
 return controlNoData(mmio,c,evt,ep0,slot,setup);
}
static bool submitInterruptIn(unsigned long mmio,const XhciCaps&c,Ring&evt,Ring&intr,unsigned int slot,unsigned int dci,const HidEndpoint&hid,DriverDmaInfo&report,unsigned int&actual){
 actual=0;unsigned int ask=hid.maxPacket;if(!ask) return false;if(ask>64)ask=64;
 if(!report.user){if(!allocDma(64,64,report))return false;}zeroDma(report);
 // Normal TRB, IOC. Doorbell target is the endpoint DCI.
 if(!queueTransfer(intr,report.physical,ask,(1u<<10)|(1u<<5)))return false;
 ringDoorbell(mmio,c,slot,dci);Trb e{};if(!waitTransferEvent(evt,slot,dci,e,12000000))return false;
 unsigned int residual=e.status&0x00ffffffu;actual=residual<ask?ask-residual:0u;return actual>0;
}
static bool forwardHidReport(unsigned long inputService,const HidEndpoint&hid,const DriverDmaInfo&report,unsigned int actual){
 if(!inputService||!report.user||!actual)return false;HidReport m{};m.protocol=hid.protocol;m.length=(unsigned char)(actual>64?64:actual);volatile unsigned char*src=(volatile unsigned char*)report.user;for(unsigned int i=0;i<m.length;i++)m.data[i]=src[i];
 return call(50,inputService,0x48494431u,(unsigned long)&m,4u+m.length)==0;
}

static bool waitBit(volatile unsigned int*r,unsigned int mask,bool set,unsigned long spins=4000000){while(spins--){bool v=((*r&mask)!=0);if(v==set)return true;asm volatile("pause");}return false;}
static bool takeAndStart(unsigned long device,unsigned long mmio,const XhciCaps&c,Ring&cmd,Ring&evt,DriverDmaInfo&erst,DriverDmaInfo&dcbaa){
 if(!c.valid||!cmd.ready||!evt.ready||!erst.user||!dcbaa.user)return false;
 // v1.87 recovery policy: keep xHCI ownership with the hardware-proven kernel compatibility path.
 // Ring-3 enumeration remains compiled, but takeover is disabled until its report path is hardware-proven.
 return false;
 unsigned long op=mmio+c.capLength;volatile unsigned int*usbcmd=reg32(op,0),*usbsts=reg32(op,4);
 *usbcmd&=~1u;if(!waitBit(usbsts,1u,true))return false; // halted
 *usbcmd|=2u;if(!waitBit(usbcmd,2u,false))return false;if(!waitBit(usbsts,1u<<11,false))return false; // reset/CNR
 *reg32(op,0x38)=c.maxSlots>255?255:c.maxSlots; // CONFIG.MaxSlotsEn
 *(volatile unsigned long*)(op+0x30)=dcbaa.physical;
 *(volatile unsigned long*)(op+0x18)=cmd.dma.physical|1ull;
 unsigned long rt=mmio+(c.rtsoff&~0x1fu),intr=rt+0x20;
 *reg32(intr,0x08)=1;*(volatile unsigned long*)(intr+0x10)=erst.physical;*(volatile unsigned long*)(intr+0x18)=evt.dma.physical|(1ull<<3);*reg32(intr,0)=2u;
 *usbcmd|=(1u|4u);if(!waitBit(usbsts,1u,false))return false;
 return true;
}
extern "C" __attribute__((section(".text.entry"))) void _davis_start(){
 unsigned long device=(unsigned long)call(72);if(!device)for(;;)call(2);if(call(64,device)!=0)for(;;)call(2);
 DriverResourceInfo r[12]{};unsigned long n=0;for(;n<12;n++){long q=call(73,n,(unsigned long)&r[n]);if(q<=0)break;}
 unsigned long irqEp=(unsigned long)call(49,2),irqHandle=0,mmioHandle=0,mmio=0,mmioLen=0;
 for(unsigned long i=0;i<n;i++)if(r[i].type==1&&!mmio){long h=call(65,r[i].id,2);if(h>0){long va=call(68,(unsigned long)h);if(va>0){mmioHandle=(unsigned long)h;mmio=(unsigned long)va;mmioLen=r[i].length;}else call(66,(unsigned long)h);}}
 XhciCaps caps=probe(mmio,mmioLen);
 for(unsigned long i=0;i<n;i++)if(r[i].type==5&&!irqHandle){long h=call(65,r[i].id,5);if(h>0&&call(76,(unsigned long)h)>0&&irqEp&&call(71,(unsigned long)h,irqEp)==0)irqHandle=(unsigned long)h;else if(h>0)call(66,(unsigned long)h);}
 // Build the actual xHCI DMA structures in the Ring-3 driver's address space.
 // Controller register ownership remains with the compatibility backend in v1.78,
 // so these structures are deliberately not installed into CRCR/DCBAAP/ERSTBA yet.
 Ring command{},event{};DriverDmaInfo erst{},dcbaa{};
 bool ringsReady=caps.valid&&initCommandRing(command)&&initEventRing(event,erst)&&allocDma(4096,64,dcbaa);
 if(ringsReady){zeroDma(dcbaa);volatile unsigned long*dc=(volatile unsigned long*)dcbaa.user;dc[0]=0;}
 bool ownsController=ringsReady&&takeAndStart(device,mmio,caps,command,event,erst,dcbaa);
 // Enumerate connected root ports non-destructively after Ring-3 owns the controller.
 unsigned int connectedMask=0,firstSlot=0,firstPort=0xffffffffu;DriverDmaInfo deviceCtx{},inputCtx{},deviceDescriptor{},configDescriptor{};Ring ep0{},hidRing{};HidEndpoint hid{};bool addressed=false,descriptorReady=false,configReady=false,endpointReady=false,configured=false;unsigned int descriptorBytes=0,configBytes=0,hidDci=0;if(ownsController){unsigned long op=mmio+caps.capLength;for(unsigned int port=0;port<caps.maxPorts&&port<32;port++){unsigned int ps=*reg32(op,0x400+port*0x10);if(ps&1u){connectedMask|=(1u<<port);if(firstPort==0xffffffffu)firstPort=port;}}if(firstPort!=0xffffffffu&&enableFirstSlot(mmio,caps,command,event,firstSlot)){addressed=addressDevice(mmio,caps,command,event,dcbaa,firstPort+1,firstSlot,deviceCtx,inputCtx,ep0);if(addressed){descriptorReady=getDeviceDescriptor(mmio,caps,event,ep0,firstSlot,deviceDescriptor,descriptorBytes);if(descriptorReady)configReady=getConfigurationDescriptor(mmio,caps,event,ep0,firstSlot,configDescriptor,configBytes,hid);if(configReady)endpointReady=configureHidEndpoint(mmio,caps,command,event,firstSlot,firstPort+1,inputCtx,hid,hidRing,hidDci);if(endpointReady)configured=setConfiguration(mmio,caps,event,ep0,firstSlot,hid.configuration);}}}
 unsigned long inputService=0;DriverDmaInfo hidReport{};bool bootProtocol=false;unsigned int hidBytes=0;
 if(configured){bootProtocol=setBootProtocol(mmio,caps,event,ep0,firstSlot,hid);long svc=call(48,3);if(svc>0)inputService=(unsigned long)svc;}
 for(;;){if(configured&&bootProtocol){if(!inputService){long svc=call(48,3);if(svc>0)inputService=(unsigned long)svc;}if(submitInterruptIn(mmio,caps,event,hidRing,firstSlot,hidDci,hid,hidReport,hidBytes))forwardHidReport(inputService,hid,hidReport,hidBytes);}call(67,device);call(2);}
 (void)inputService;(void)hidReport;(void)bootProtocol;(void)hidBytes;(void)ringsReady;(void)ownsController;(void)connectedMask;(void)firstSlot;(void)firstPort;(void)addressed;(void)descriptorReady;(void)descriptorBytes;(void)deviceDescriptor;(void)configReady;(void)configBytes;(void)configDescriptor;(void)endpointReady;(void)configured;(void)hid;(void)hidRing;(void)hidDci;(void)deviceCtx;(void)inputCtx;(void)ep0;(void)irqHandle;(void)mmioHandle;
}
extern "C" void* memset(void* d,int v,unsigned long n){unsigned char*p=(unsigned char*)d;while(n--)*p++=(unsigned char)v;return d;}
