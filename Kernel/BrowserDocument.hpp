#pragma once
#include "../Base/Types.hpp"
namespace Davis::BrowserDocument {
static constexpr u32 MaxTitle=128,MaxText=8192,MaxLinks=32,MaxLinkText=96,MaxHref=256,MaxNodes=192,MaxTag=12,MaxClass=48,MaxId=32,MaxInlineStyle=160,MaxCss=4096;
enum class NodeType:u8{Text,Paragraph,Heading1,Heading2,Heading3,ListItem,Link};
struct Node{NodeType type;u32 textOffset,textBytes;u16 linkIndex;bool valid;char tag[MaxTag],klass[MaxClass],id[MaxId],inlineStyle[MaxInlineStyle];};
struct Link{char text[MaxLinkText];char href[MaxHref];u32 textOffset;bool valid;};
struct Document{char title[MaxTitle];char text[MaxText];u32 textBytes;Node nodes[MaxNodes];u32 nodeCount;Link links[MaxLinks];u32 linkCount;char css[MaxCss];u32 cssBytes;u64 parses;bool valid,truncated;};
extern Document state;
void Init(); bool Parse(const char* html,u32 bytes); const char* Title();
}
