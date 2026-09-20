#include "BrowserResources.hpp"
#include "BrowserCore.hpp"
namespace Davis::BrowserResources {
State state{};
static bool ci(char a,char b){if(a>='A'&&a<='Z')a+=32;if(b>='A'&&b<='Z')b+=32;return a==b;}
static bool at(const char*d,u32 n,u32 p,const char*s){u32 k=0;while(s[k]){if(p+k>=n||!ci(d[p+k],s[k]))return false;k++;}return true;}
static u32 attr(const char*d,u32 n,u32 p,const char*name,char*out,u32 cap){for(;p<n&&d[p]!='>';p++){if(!at(d,n,p,name))continue;if(p&&d[p-1]!=' '&&d[p-1]!='\t'&&d[p-1]!='\n'&&d[p-1]!='\r')continue;u32 q=p,k=0;while(name[k])q++,k++;while(q<n&&d[q]==' ')q++;if(q>=n||d[q++]!='=')continue;while(q<n&&d[q]==' ')q++;char quote=0;if(q<n&&(d[q]=='\''||d[q]=='"'))quote=d[q++];u32 z=0;while(q<n&&z+1<cap&&((quote&&d[q]!=quote)||(!quote&&d[q]!=' '&&d[q]!='>')))out[z++]=d[q++];out[z]=0;return z;}if(cap)out[0]=0;return 0;}
static void add(Kind kind,const char*ref,const char*base){if(!ref||!ref[0])return;if(state.count>=MaxResources){state.truncated=true;return;}char resolved[MaxUrl]{};Browser::Url b{};if(!Browser::ParseUrl(base,b)||!Browser::ResolveUrl(b,ref,resolved,sizeof(resolved)))return;for(u32 i=0;i<state.count;i++){u32 j=0;while(state.items[i].url[j]&&resolved[j]&&state.items[i].url[j]==resolved[j])j++;if(!state.items[i].url[j]&&!resolved[j])return;}Resource&r=state.items[state.count++];r={};r.kind=kind;r.phase=Phase::Queued;r.valid=true;u32 i=0;while(resolved[i]&&i+1<MaxUrl){r.url[i]=resolved[i];i++;}r.url[i]=0;state.discoveries++;}
void Init(){state={};}
void Discover(const char*d,u32 n,const char*base){Init();if(!d||!base)return;for(u32 i=0;i<n;i++){if(d[i]!='<')continue;u32 p=i+1;while(p<n&&d[p]==' ')p++;if(at(d,n,p,"img")){char src[MaxUrl]{};if(attr(d,n,p,"src",src,sizeof(src)))add(Kind::Image,src,base);}else if(at(d,n,p,"link")){char rel[48]{},href[MaxUrl]{};attr(d,n,p,"rel",rel,sizeof(rel));attr(d,n,p,"href",href,sizeof(href));bool sheet=false;for(u32 z=0;rel[z];z++)if(at(rel,48,z,"stylesheet")){sheet=true;break;}if(sheet&&href[0])add(Kind::Stylesheet,href,base);}while(i<n&&d[i]!='>')i++;}}
}
