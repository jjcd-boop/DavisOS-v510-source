static inline long call(unsigned long n,unsigned long a=0,unsigned long b=0,unsigned long c=0,unsigned long d=0){register unsigned long rax asm("rax")=n;register unsigned long rdi asm("rdi")=a;register unsigned long rsi asm("rsi")=b;register unsigned long rdx asm("rdx")=c;register unsigned long r10 asm("r10")=d;asm volatile("int $0x80":"+a"(rax):"D"(rdi),"S"(rsi),"d"(rdx),"r"(r10):"memory","cc");return (long)rax;}
struct DriverDmaInfo{unsigned long user,physical,bytes;};
struct DriverResourceInfo{unsigned long id,base,length;unsigned int type,rights,reserved;};
struct Wire{unsigned long sender;unsigned int type,bytes;unsigned char payload[192];};
struct HidReport{unsigned char protocol,length,reserved0,reserved1,data[64];};
struct Header{unsigned short major,minor;unsigned int bytes;unsigned long replyHandle,requestId;};
struct InputInjection{Header h;unsigned char protocol,length;unsigned short reserved;unsigned char data[64];};
extern "C" __attribute__((section(".text.entry"))) void _davis_start(){
 unsigned long device=(unsigned long)call(72);if(!device)for(;;)call(2);if(call(64,device)!=0)for(;;)call(2);
 DriverResourceInfo resources[8]{};unsigned long resourceCount=0;for(unsigned long i=0;i<8;i++){long r=call(73,i,(unsigned long)&resources[i]);if(r<=0)break;resourceCount++;}
 unsigned long irqEndpoint=(unsigned long)call(49,2),irqHandle=0;for(unsigned long i=0;i<resourceCount;i++)if(resources[i].type==5){long h=call(65,resources[i].id,5);if(h>0&&call(76,(unsigned long)h)>0&&irqEndpoint&&call(71,(unsigned long)h,irqEndpoint)==0){irqHandle=(unsigned long)h;break;}}
 // Ring-3 input service endpoint. UsbDriver looks this up and sends bounded HID reports here.
 unsigned long inputEndpoint=(unsigned long)call(49,3);bool serviceReady=inputEndpoint&&call(53,3,inputEndpoint)==0;unsigned long desktop=(unsigned long)call(48,1);
 DriverDmaInfo dma{};if(call(74,4096,4096,(unsigned long)&dma)==0&&dma.user)call(75,dma.user);
 unsigned long reports=0;for(;;){
  if(serviceReady){for(unsigned int budget=0;budget<8;budget++){Wire w{};long n=call(51,inputEndpoint,(unsigned long)&w);if(n<=0)break;if(w.type!=0x48494431u||w.bytes<4)continue;HidReport*r=(HidReport*)w.payload;if(r->length>64||w.bytes<4u+r->length)continue;
    // v1.84 owns HID decoding in Ring 3. Generic desktop injection is intentionally
    // deferred until the Ring-3 input queue ABI is installed; raw reports never enter Ring 0.
    if((r->protocol==1&&r->length>=8)||(r->protocol==2&&r->length>=3)){reports++;if(!desktop)desktop=(unsigned long)call(48,1);if(desktop){InputInjection e{};e.h={1,1,sizeof(e),0,reports};e.protocol=r->protocol;e.length=r->length;for(unsigned int j=0;j<r->length;j++)e.data[j]=r->data[j];call(50,desktop,0x44030001u,(unsigned long)&e,sizeof(e));}}
  }}
  call(67,device);call(2);
 }
 (void)irqHandle;(void)reports;
}
