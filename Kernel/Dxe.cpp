#include "Dxe.hpp"
namespace Davis::Dxe {
static bool addOk(u64 a,u64 b,u64& out){out=a+b;return out>=a;}
static bool range(u64 off,u64 n,u64 total){u64 end=0;return addOk(off,n,end)&&off<=total&&end<=total;}
static bool overlap(u64 a,u64 an,u64 b,u64 bn){if(!an||!bn)return false;u64 ae=0,be=0;if(!addOk(a,an,ae)||!addOk(b,bn,be))return true;return a<be&&b<ae;}
ValidateResult Validate(const void* image,u64 bytes,const Header** out){
 if(out)*out=nullptr;if(!image)return ValidateResult::NullImage;if(bytes<sizeof(Header))return ValidateResult::TooSmall;
 const auto*h=(const Header*)image;if(h->magic!=Magic)return ValidateResult::BadMagic;
 if(h->headerBytes<sizeof(Header)||h->headerBytes>bytes||(h->headerBytes&15u))return ValidateResult::BadHeader;
 if(h->abiMajor!=AbiMajor||h->abiMinor>AbiMinor)return ValidateResult::UnsupportedAbi;
 if((h->flags&(UserMode|PositionIndependent))!=(UserMode|PositionIndependent)||(h->flags&~KnownFlags))return ValidateResult::BadFlags;
 if(h->reserved0)return ValidateResult::ReservedNotZero;for(u64 v:h->reserved)if(v)return ValidateResult::ReservedNotZero;
 if(h->imageBytes!=bytes)return ValidateResult::BadImageSize;if(!h->codeBytes)return ValidateResult::BadCodeRange;
 if(!range(h->codeOffset,h->codeBytes,bytes)||h->codeOffset<h->headerBytes)return ValidateResult::BadCodeRange;
 if(h->dataBytes&&(!range(h->dataOffset,h->dataBytes,bytes)||h->dataOffset<h->headerBytes))return ValidateResult::BadDataRange;
 if(overlap(h->codeOffset,h->codeBytes,h->dataOffset,h->dataBytes))return ValidateResult::OverlappingSegments;
 u64 codeEnd=0;if(!addOk(h->codeOffset,h->codeBytes,codeEnd))return ValidateResult::ArithmeticOverflow;
 if(h->entryOffset<h->codeOffset||h->entryOffset>=codeEnd)return ValidateResult::BadEntry;
 if((h->codeOffset&15u)||(h->dataBytes&&(h->dataOffset&15u)))return ValidateResult::BadAlignment;
 const u64 stack=h->requiredStackBytes?h->requiredStackBytes:65536;if(stack<MinStackBytes||stack>MaxStackBytes)return ValidateResult::BadStackSize;
 u64 db=0;if(!addOk(h->dataBytes,h->bssBytes,db))return ValidateResult::ArithmeticOverflow;
 if(out)*out=h;return ValidateResult::Ok;
}
const char* ResultName(ValidateResult r){switch(r){case ValidateResult::Ok:return"Ok";case ValidateResult::NullImage:return"NullImage";case ValidateResult::TooSmall:return"TooSmall";case ValidateResult::BadMagic:return"BadMagic";case ValidateResult::BadHeader:return"BadHeader";case ValidateResult::UnsupportedAbi:return"UnsupportedAbi";case ValidateResult::BadFlags:return"BadFlags";case ValidateResult::BadImageSize:return"BadImageSize";case ValidateResult::BadCodeRange:return"BadCodeRange";case ValidateResult::BadDataRange:return"BadDataRange";case ValidateResult::BadEntry:return"BadEntry";case ValidateResult::BadAlignment:return"BadAlignment";case ValidateResult::OverlappingSegments:return"OverlappingSegments";case ValidateResult::BadStackSize:return"BadStackSize";case ValidateResult::ReservedNotZero:return"ReservedNotZero";default:return"ArithmeticOverflow";}}
}
