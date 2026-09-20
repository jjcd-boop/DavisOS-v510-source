#pragma once
#include "../Base/Types.hpp"
#include "BrowserDocument.hpp"
namespace Davis::BrowserStyle {
static constexpr u32 MaxRules=64,MaxSelector=48,MaxDeclarations=192;
struct Style { u32 foreground,background,borderColor; i16 marginTop,marginBottom,paddingLeft,paddingRight,width,height,borderWidth; u8 scale; bool underline; };
struct Rule { char selector[MaxSelector]; char declarations[MaxDeclarations]; u16 specificity,order; bool valid; };
struct Sheet { Rule rules[MaxRules]; u32 count; bool truncated; };
extern Sheet sheet;
void Init(); void ParseStylesheet(const char* css,u32 bytes); Style ForNode(BrowserDocument::NodeType type); Style Compute(const BrowserDocument::Node& node);
}
