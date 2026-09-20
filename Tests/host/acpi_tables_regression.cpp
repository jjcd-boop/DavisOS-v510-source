#include "Kernel/AcpiTables.hpp"
#include <cstdio>
#include <cstring>
using namespace Davis;
#pragma pack(push,1)
struct R { char sig[8]; u8 checksum; char oem[6]; u8 revision; u32 rsdt; u32 length; u64 xsdt; u8 extChecksum; u8 reserved[3]; };
struct H { char sig[4]; u32 length; u8 revision,checksum; char oem[6]; char tableId[8]; u32 oemRev,creatorId,creatorRev; };
struct X { H h; u64 entries[2]; };
struct F { H h; u32 firmwareCtrl; u32 dsdt; u8 pad[104]; u64 xdsdt; };
#pragma pack(pop)
static void fix(void* p, size_t n, size_t off){auto*b=(u8*)p;b[off]=0;u8 sum=0;for(size_t i=0;i<n;i++)sum=(u8)(sum+b[i]);b[off]=(u8)(0-sum);}
static void head(H&h,const char*s,u32 n){std::memset(&h,0,sizeof(h));std::memcpy(h.sig,s,4);h.length=n;h.revision=2;std::memcpy(h.oem,"DAVIS ",6);}
int main(){
 F f{};head(f.h,"FACP",sizeof(f));f.dsdt=0x12345000;fix(&f,sizeof(f),9);
 H bad{};head(bad,"SSDT",sizeof(bad));bad.checksum=1;
 X x{};head(x.h,"XSDT",sizeof(x));x.entries[0]=(u64)(uptr)&f;x.entries[1]=(u64)(uptr)&bad;fix(&x,sizeof(x),9);
 R r{};std::memcpy(r.sig,"RSD PTR ",8);std::memcpy(r.oem,"DAVIS ",6);r.revision=2;r.length=sizeof(r);r.xsdt=(u64)(uptr)&x;fix(&r,20,8);fix(&r,sizeof(r),32);
 BootInfo bi{};bi.rsdp=(uptr)&r;Acpi::State st{};Acpi::Discover(bi,st);
 if(!st.rsdpValid||!st.xsdtPresent||!st.fadtPresent||!st.dsdtPresent||st.count!=1||!Acpi::Find(st,"FACP")||Acpi::Find(st,"SSDT")) return 1;
 r.extChecksum++;Acpi::Discover(bi,st);if(st.rsdpValid) return 2;
 std::puts("PASS acpi checksums xsdt fadt dsdt corrupt-table-reject");return 0;
}
