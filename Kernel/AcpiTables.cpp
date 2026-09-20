#include "AcpiTables.hpp"
namespace Davis::Acpi {
#pragma pack(push,1)
struct Rsdp { char sig[8]; u8 checksum; char oem[6]; u8 revision; u32 rsdt; u32 length; u64 xsdt; u8 extChecksum; u8 reserved[3]; };
struct Sdt { char sig[4]; u32 length; u8 revision,checksum; char oem[6]; char tableId[8]; u32 oemRev,creatorId,creatorRev; };
struct FadtPrefix { Sdt h; u32 firmwareCtrl; u32 dsdt; };
#pragma pack(pop)
static bool sum0(const void*p,u32 n){const volatile u8*b=(const volatile u8*)p;u8 s=0;for(u32 i=0;i<n;i++)s=(u8)(s+b[i]);return s==0;}
static bool eq4(const char*a,const char*b){return a[0]==b[0]&&a[1]==b[1]&&a[2]==b[2]&&a[3]==b[3];}
static void add(State&s,uptr a){if(!a||s.count>=32)return;auto*h=(const Sdt*)a;if(h->length<sizeof(Sdt)||h->length>(16u<<20))return;if(!sum0(h,h->length))return;auto&t=s.tables[s.count++];for(int i=0;i<4;i++)t.sig[i]=h->sig[i];t.sig[4]=0;t.address=a;t.length=h->length;t.checksumOk=true;if(eq4(h->sig,"FACP")){s.fadtPresent=true;s.fadt=a;auto*f=(const FadtPrefix*)h;if(f->dsdt){s.dsdtPresent=true;s.dsdt=f->dsdt;}}}
void Discover(const BootInfo&b,State&s){s={};if(!b.rsdp)return;auto*r=(const Rsdp*)b.rsdp;if(!sum0(r,20))return;if(r->revision>=2&&r->length>=36&&!sum0(r,r->length))return;s.rsdpValid=true;uptr root=0;bool xs=false;if(r->revision>=2&&r->xsdt){root=(uptr)r->xsdt;xs=true;s.xsdtPresent=true;}else if(r->rsdt){root=(uptr)r->rsdt;s.rsdtPresent=true;}if(!root)return;auto*h=(const Sdt*)root;if(h->length<sizeof(Sdt)||h->length>(4u<<20)||!sum0(h,h->length))return;s.root=root;u32 es=xs?8:4,n=(h->length-sizeof(Sdt))/es;if(n>128)n=128;const u8*p=(const u8*)h+sizeof(Sdt);for(u32 i=0;i<n;i++){uptr a=xs?(uptr)(*(const u64*)(p+i*8)):(uptr)(*(const u32*)(p+i*4));add(s,a);}if(s.fadtPresent){auto*f=(const u8*)s.fadt;auto*h2=(const Sdt*)f;if(h2->length>=148){u64 x=*(const u64*)(f+140);if(x){s.dsdt=x;s.dsdtPresent=true;}}}}
const TableInfo* Find(const State&s,const char sig[4]){for(u32 i=0;i<s.count;i++)if(eq4(s.tables[i].sig,sig))return&s.tables[i];return nullptr;}
}
