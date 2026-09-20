#include "BrowserDocument.hpp"
#include "BrowserStyle.hpp"
namespace Davis::BrowserDocument {
Document state{};
static bool ci(char a,char b){if(a>='A'&&a<='Z')a+=32;if(b>='A'&&b<='Z')b+=32;return a==b;}
static bool at(const char*d,u32 n,u32 p,const char*s){u32 k=0;while(s[k]){if(p+k>=n||!ci(d[p+k],s[k]))return false;k++;}return true;}
static void copy(char*out,u32 cap,const char*in){u32 i=0;while(in&&in[i]&&i+1<cap){out[i]=in[i];i++;}out[i]=0;}
static void add(char c){if(state.textBytes+1>=MaxText){state.truncated=true;return;}if(c=='\r'||c=='\n'||c=='\t')c=' ';if(c==' '&&state.textBytes&&state.text[state.textBytes-1]==' ')return;state.text[state.textBytes++]=c;state.text[state.textBytes]=0;}
static void space(){if(state.textBytes&&state.text[state.textBytes-1]!=' ')add(' ');}
static u32 attr(const char*d,u32 n,u32 p,const char*name,char*out,u32 cap){for(;p<n&&d[p]!='>';p++){if(!at(d,n,p,name))continue;if(p>0){char prev=d[p-1];if(!((prev==' ')||(prev=='\t')||(prev=='\n')||(prev=='\r')))continue;}u32 q=p,k=0;while(name[k])q++,k++;while(q<n&&d[q]==' ')q++;if(q>=n||d[q]!='=')continue;q++;while(q<n&&d[q]==' ')q++;char quote=0;if(q<n&&(d[q]=='\''||d[q]=='"'))quote=d[q++];u32 z=0;while(q<n&&z+1<cap&&((quote&&d[q]!=quote)||(!quote&&d[q]!=' '&&d[q]!='>')))out[z++]=d[q++];out[z]=0;return z;}if(cap)out[0]=0;return 0;}
static void tagName(const char*d,u32 n,u32 p,char*out){u32 z=0;while(p<n&&z+1<MaxTag){char c=d[p];if(c==' '||c=='\t'||c=='\r'||c=='\n'||c=='>'||c=='/')break;out[z++]=(c>='A'&&c<='Z')?c+32:c;p++;}out[z]=0;}
static void node(NodeType t,u32 start,u16 link,const char*tag,const char*klass,const char*id,const char*style){if(start>=state.textBytes||state.nodeCount>=MaxNodes)return;Node&x=state.nodes[state.nodeCount++];x={};x.type=t;x.textOffset=start;x.textBytes=state.textBytes-start;x.linkIndex=link;x.valid=true;copy(x.tag,MaxTag,tag);copy(x.klass,MaxClass,klass);copy(x.id,MaxId,id);copy(x.inlineStyle,MaxInlineStyle,style);}
void Init(){state={};BrowserStyle::Init();}
bool Parse(const char*d,u32 n){u64 old=state.parses;state={};state.parses=old+1;BrowserStyle::Init();if(!d||!n)return false;bool script=false,styleTag=false,title=false,anchor=false;u32 li=0,blockStart=0;NodeType block=NodeType::Text;char btag[MaxTag]{},bclass[MaxClass]{},bid[MaxId]{},bstyle[MaxInlineStyle]{};char atag[MaxTag]{"a"},aclass[MaxClass]{},aid[MaxId]{},astyle[MaxInlineStyle]{};
for(u32 i=0;i<n;){if(d[i]=='<'){if(at(d,n,i,"<!--")){while(i+2<n&&!(d[i]=='-'&&d[i+1]=='-'&&d[i+2]=='>'))i++;i+=i+2<n?3:0;continue;}bool close=i+1<n&&d[i+1]=='/';u32 p=i+(close?2:1);while(p<n&&d[p]==' ')p++;char tn[MaxTag]{};tagName(d,n,p,tn);
 if(at(d,n,p,"script"))script=!close;if(at(d,n,p,"style")){if(close&&styleTag){BrowserStyle::ParseStylesheet(state.css,state.cssBytes);}styleTag=!close;}if(at(d,n,p,"title"))title=!close;
 bool isBlock=at(d,n,p,"p")||at(d,n,p,"div")||at(d,n,p,"h1")||at(d,n,p,"h2")||at(d,n,p,"h3")||at(d,n,p,"li");
 if(!close&&isBlock){space();blockStart=state.textBytes;block=at(d,n,p,"h1")?NodeType::Heading1:at(d,n,p,"h2")?NodeType::Heading2:at(d,n,p,"h3")?NodeType::Heading3:at(d,n,p,"li")?NodeType::ListItem:NodeType::Paragraph;copy(btag,MaxTag,tn);attr(d,n,p,"class",bclass,MaxClass);attr(d,n,p,"id",bid,MaxId);attr(d,n,p,"style",bstyle,MaxInlineStyle);}
 if(close&&isBlock){node(block,blockStart,0xFFFF,btag,bclass,bid,bstyle);space();}
 if(at(d,n,p,"br"))space();
 if(at(d,n,p,"a")&&!close&&state.linkCount<MaxLinks){li=state.linkCount++;state.links[li]={};state.links[li].textOffset=state.textBytes;state.links[li].valid=attr(d,n,p,"href",state.links[li].href,MaxHref)>0;copy(atag,MaxTag,"a");attr(d,n,p,"class",aclass,MaxClass);attr(d,n,p,"id",aid,MaxId);attr(d,n,p,"style",astyle,MaxInlineStyle);anchor=true;}
 if(at(d,n,p,"a")&&close&&anchor){u32 start=state.links[li].textOffset,z=0;while(start<state.textBytes&&z+1<MaxLinkText)state.links[li].text[z++]=state.text[start++];state.links[li].text[z]=0;node(NodeType::Link,state.links[li].textOffset,(u16)li,atag,aclass,aid,astyle);anchor=false;}
 while(i<n&&d[i]!='>')i++;if(i<n)i++;continue;}
 if(styleTag&&!script){if(state.cssBytes+1<MaxCss){state.css[state.cssBytes++]=d[i];state.css[state.cssBytes]=0;}else state.truncated=true;}
 else if(!script&&!styleTag){if(title){u32 z=0;while(z+1<MaxTitle&&state.title[z])z++;if(z+1<MaxTitle)state.title[z]=d[i],state.title[z+1]=0;}else add(d[i]);}i++;}
 if(!state.title[0])copy(state.title,MaxTitle,"Untitled page");if(!state.nodeCount&&state.textBytes)node(NodeType::Paragraph,0,0xFFFF,"p","","","");state.valid=true;return true;}
const char* Title(){return state.title[0]?state.title:"Davis Browser";}
}
