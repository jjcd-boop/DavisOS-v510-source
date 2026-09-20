#pragma once
#include "BootInfo.hpp"
namespace Davis::Acpi {
struct TableInfo { char sig[5]; uptr address; u32 length; bool checksumOk; };
struct State { bool rsdpValid,xsdtPresent,rsdtPresent,fadtPresent,dsdtPresent; uptr root,fadt,dsdt; u32 count; TableInfo tables[32]; };
void Discover(const BootInfo&,State&);
const TableInfo* Find(const State&,const char sig[4]);
}
