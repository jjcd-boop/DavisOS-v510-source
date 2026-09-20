#include "UserMemory.hpp"
namespace Davis::UserMemory {
static void cp(u8*d,const u8*s,u64 n){for(u64 i=0;i<n;i++)d[i]=s[i];}
static bool canonicalUser(u64 a){return a<0x0000800000000000ull;}
static Paging::Space spaceOf(const Process::Image&p){return {p.addressSpaceCr3,0,p.addressSpaceCr3!=0};}
Result ValidateRange(const Process::Image&p,u64 va,u64 n,Access access){
 if(!p.addressSpaceCr3)return Result::InvalidProcess;if(!n)return Result::Ok;if(!va)return Result::NullPointer;
 if(va>~0ull-(n-1))return Result::Overflow;u64 last=va+n-1;if(!canonicalUser(va)||!canonicalUser(last))return Result::Supervisor;
 auto s=spaceOf(p);u64 cur=va;while(cur<=last){u64 pa=0,e=0;if(!Paging::Translate(s,cur,&pa,&e))return Result::Unmapped;if(!(e&Paging::U))return Result::Supervisor;if(access==Access::Write&&!(e&Paging::W))return Result::ReadOnly;u64 next=(cur&~4095ull)+4096;if(next==0||next>last)break;cur=next;}return Result::Ok;
}
Result CopyFromUser(const Process::Image&p,void*dst,u64 src,u64 n){if(n&&!dst)return Result::NullPointer;auto r=ValidateRange(p,src,n,Access::Read);if(r!=Result::Ok)return r;auto s=spaceOf(p);u8*out=(u8*)dst;u64 done=0;while(done<n){u64 pa=0;if(!Paging::Translate(s,src+done,&pa,nullptr))return Result::Unmapped;u64 chunk=4096-((src+done)&4095);if(chunk>n-done)chunk=n-done;cp(out+done,(const u8*)(uptr)pa,chunk);done+=chunk;}return Result::Ok;}
Result CopyToUser(const Process::Image&p,u64 dst,const void*src,u64 n){if(n&&!src)return Result::NullPointer;auto r=ValidateRange(p,dst,n,Access::Write);if(r!=Result::Ok)return r;auto s=spaceOf(p);const u8*in=(const u8*)src;u64 done=0;while(done<n){u64 pa=0;if(!Paging::Translate(s,dst+done,&pa,nullptr))return Result::Unmapped;u64 chunk=4096-((dst+done)&4095);if(chunk>n-done)chunk=n-done;cp((u8*)(uptr)pa,in+done,chunk);done+=chunk;}return Result::Ok;}
Result CopyStringFromUser(const Process::Image&p,char*dst,u64 cap,u64 src,u64*len){if(!dst||cap<2||!src)return Result::NullPointer;for(u64 i=0;i<cap-1;i++){char c=0;auto r=CopyFromUser(p,&c,src+i,1);if(r!=Result::Ok)return r;dst[i]=c;if(!c){if(len)*len=i;return Result::Ok;}}dst[cap-1]=0;if(len)*len=cap-1;return Result::Overflow;}
}
