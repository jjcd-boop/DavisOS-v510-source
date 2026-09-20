#include "Kernel/AcpiTables.hpp"
#include "Kernel/AcpiMadt.hpp"
#include <cstdio>
#include <cstring>
using namespace Davis;
#pragma pack(push,1)
struct R { char sig[8]; u8 checksum; char oem[6]; u8 revision; u32 rsdt; u32 length; u64 xsdt; u8 extChecksum; u8 reserved[3]; };
struct H { char sig[4]; u32 length; u8 revision,checksum; char oem[6]; char tableId[8]; u32 oemRev,creatorId,creatorRev; };
struct X { H h; u64 entries[1]; };
struct M { H h; u32 lapic,flags; u8 data[64]; };
#pragma pack(pop)
static void fix(void*p,size_t n,size_t off){auto*b=(u8*)p;b[off]=0;u8 s=0;for(size_t i=0;i<n;i++)s=(u8)(s+b[i]);b[off]=(u8)(0-s);}
static void head(H&h,const char*s,u32 n){std::memset(&h,0,sizeof(h));std::memcpy(h.sig,s,4);h.length=n;h.revision=5;std::memcpy(h.oem,"DAVIS ",6);}
int main(){M m{};head(m.h,"APIC",44);m.lapic=0xfee00000;m.flags=1;u8*p=m.data;
 // CPU0 enabled, CPU1 enabled.
 u8 c0[]={0,8,0,0,1,0,0,0},c1[]={0,8,1,1,1,0,0,0};std::memcpy(p,c0,8);p+=8;std::memcpy(p,c1,8);p+=8;
 // IOAPIC id 2 at fec00000, GSI base 0.
 u8 io[]={1,12,2,0,0,0,0xc0,0xfe,0,0,0,0};std::memcpy(p,io,12);p+=12;
 // IRQ0 override -> GSI2, active high/edge defaults encoded zero.
 u8 iso[]={2,10,0,0,2,0,0,0,0,0};std::memcpy(p,iso,10);p+=10;m.h.length=(u32)((u8*)p-(u8*)&m);fix(&m,m.h.length,9);
 X x{};head(x.h,"XSDT",sizeof(x));x.entries[0]=(u64)(uptr)&m;fix(&x,sizeof(x),9);R r{};std::memcpy(r.sig,"RSD PTR ",8);std::memcpy(r.oem,"DAVIS ",6);r.revision=2;r.length=sizeof(r);r.xsdt=(u64)(uptr)&x;fix(&r,20,8);fix(&r,sizeof(r),32);BootInfo bi{};bi.rsdp=(uptr)&r;Acpi::State ac{};Acpi::Discover(bi,ac);AcpiMadt::State s{};if(!AcpiMadt::Parse(ac,s)||s.cpuCount!=2||s.enabledCpuCount!=2||s.ioApicCount!=1||s.overrideCount!=1||s.localApicAddress!=0xfee00000)return 1;u32 g=0;u16 f=99;if(!AcpiMadt::ResolveIsaIrq(s,0,g,f)||g!=2||f!=0)return 2;if(!AcpiMadt::ResolveIsaIrq(s,1,g,f)||g!=1)return 3;
 // Malformed entry length must fail closed even with a valid table checksum.
 m.data[1]=1;fix(&m,m.h.length,9);Acpi::Discover(bi,ac);if(AcpiMadt::Parse(ac,s))return 4;std::puts("PASS madt cpu=2 ioapic=1 iso=1 malformed-reject");return 0;}
