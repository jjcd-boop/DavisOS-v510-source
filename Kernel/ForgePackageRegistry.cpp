#include "ForgePackageRegistry.hpp"
#include "ForgePersistentStore.hpp"
#include "MultiAppRuntime.hpp"
#include "BootInfo.hpp"
namespace Davis::ForgePackageRegistry {
static constexpr u32 Magic=0x33475046; // FPG3
static Package cache[MaxPackages]{};static bool used[MaxPackages]{};
struct Pending { u64 pid; Package pkg; bool used; }; static Pending pending[8]{};
static u32 key(u32 s){return 0xF3000000u+s;}
static u32 hash(const Package&p){u32 h=2166136261u;const u8*b=(const u8*)&p;for(u32 i=0;i<sizeof(Package)-128+p.bytes;i++){if(i>=20&&i<24)continue;h^=b[i];h*=16777619u;}return h;}
static bool valid(const Package&p){return p.magic==Magic&&p.version==0x00030080&&p.bytes&&p.bytes<=128&&(p.bytes%4)==0&&p.budget&&p.budget<=256&&(p.caps&~3u)==0&&p.digest==hash(p);}
void Init(){for(auto &p:pending)p={};for(u32 i=0;i<MaxPackages;i++){used[i]=false;Package p{};i64 n=ForgePersistentStore::Read(key(i),&p,sizeof(p));if(n==(i64)sizeof(p)&&valid(p)){cache[i]=p;used[i]=true;}}}
bool Install(const Package&p){if(!valid(p))return false;u32 slot=MaxPackages;for(u32 i=0;i<MaxPackages;i++)if(used[i]&&cache[i].appKey==p.appKey){slot=i;break;}if(slot==MaxPackages)for(u32 i=0;i<MaxPackages;i++)if(!used[i]){slot=i;break;}if(slot==MaxPackages)return false;cache[slot]=p;used[slot]=true;ForgePersistentStore::Write(key(slot),&cache[slot],sizeof(Package));return true;}
bool Read(u32 s,Package&o){if(s>=MaxPackages||!used[s])return false;o=cache[s];return true;}
bool Remove(u32 s){if(s>=MaxPackages||!used[s])return false;used[s]=false;cache[s]={};ForgePersistentStore::Write(key(s),&cache[s],sizeof(Package));return true;}
u32 Count(){u32 n=0;for(bool x:used)if(x)n++;return n;}
bool Launch(u32 slot){if(slot>=MaxPackages||!used[slot]||!valid(cache[slot]))return false;Pending*freeSlot=nullptr;for(auto &p:pending)if(!p.used){freeSlot=&p;break;}if(!freeSlot)return false;u64 pid=MultiAppRuntime::SpawnIsolatedFromKind(AppForgeRunner);if(!pid)return false;freeSlot->pid=pid;freeSlot->pkg=cache[slot];freeSlot->used=true;return true;}
bool FetchLaunch(u64 pid,Package&out){for(auto &p:pending)if(p.used&&p.pid==pid){out=p.pkg;p.used=false;return true;}return false;}
}
