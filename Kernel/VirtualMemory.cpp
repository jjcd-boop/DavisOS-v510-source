#include "VirtualMemory.hpp"
namespace Davis::Vm {
void Init(AddressSpace&a){a={};}
static bool ov(u64 a,u64 n,u64 b,u64 m){return a<b+m&&b<a+n;}
bool AddRegion(AddressSpace&a,u64 va,u64 pa,u64 pages,u64 flags){if(!pages||(va&(PageSize-1))||(pa&(PageSize-1))||va<UserBase||va+pages*PageSize>UserTop||a.count>=AddressSpace::MaxRegions)return false;for(usize i=0;i<a.count;i++)if(ov(va,pages*PageSize,a.regions[i].virtualBase,a.regions[i].pages*PageSize))return false;a.regions[a.count++]={va,pa,pages,flags|Present|User};return true;}
bool Validate(const AddressSpace&a){for(usize i=0;i<a.count;i++){auto&r=a.regions[i];if(!(r.flags&Present)||!(r.flags&User)||!r.pages)return false;}return true;}
}
