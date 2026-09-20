#include "ApRendezvous.hpp"
namespace Davis::ApRendezvous {
static u32 load32(const volatile u32*p){return __atomic_load_n(p,__ATOMIC_ACQUIRE);} static u64 load64(const volatile u64*p){return __atomic_load_n(p,__ATOMIC_ACQUIRE);}
static void store32(volatile u32*p,u32 v){__atomic_store_n(p,v,__ATOMIC_RELEASE);} static void store64(volatile u64*p,u64 v){__atomic_store_n(p,v,__ATOMIC_RELEASE);}
bool Init(Mailbox&m,u32 n){m={};if(!n||n>MaxCpus)return false;m.count=n;m.ready=true;return true;}
bool Publish(Mailbox&m,u32 l,u32 a,u64 t,u64 g,Stage s){if(!m.ready||l>=m.count||!t||!g||s==Stage::Empty)return false;auto&x=m.slots[l];u32 old=load32(&x.stage);if(old==(u32)Stage::Failed||old>(u32)s)return false;if(old&& (load32(&x.apicId)!=a||load64(&x.token)!=t||load64(&x.generation)!=g))return false;store32(&x.apicId,a);store64(&x.token,t);store64(&x.generation,g);store32(&x.stage,(u32)s);return true;}
bool Observe(const Mailbox&m,u32 l,u32 a,u64 t,u64 g,Stage min){if(!m.ready||l>=m.count)return false;const auto&x=m.slots[l];u32 s=load32(&x.stage);return s!=(u32)Stage::Failed&&s>=(u32)min&&load32(&x.apicId)==a&&load64(&x.token)==t&&load64(&x.generation)==g;}
bool Fail(Mailbox&m,u32 l,u32 a,u64 t,u64 g){return Publish(m,l,a,t,g,Stage::Failed);}
}
