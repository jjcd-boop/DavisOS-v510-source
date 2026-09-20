#include "Dxe2.hpp"
namespace Davis::Dxe2 {
static bool add(u64 a,u64 b,u64& o){o=a+b;return o>=a;}
static bool range(u64 o,u64 n,u64 total){u64 e=0;return add(o,n,e)&&e<=total;}
static bool ov(u64 a,u64 an,u64 b,u64 bn){u64 ae=0,be=0;if(!add(a,an,ae)||!add(b,bn,be))return true;return an&&bn&&a<be&&b<ae;}
ValidateResult Validate(const void*img,u64 bytes,const Header**out){
 if(out)*out=nullptr;if(!img||bytes<sizeof(Header))return ValidateResult::BadHeader;auto*h=(const Header*)img;
 if(h->base.magic!=Magic)return ValidateResult::NotDxe2;if(h->base.headerBytes<sizeof(Header)||(h->base.headerBytes&15u)||h->base.headerBytes>bytes)return ValidateResult::BadHeader;
 if(h->base.abiMajor!=AbiMajor||h->base.abiMinor>AbiMinor)return ValidateResult::BadAbi;if(h->base.imageBytes!=bytes)return ValidateResult::BadHeader;
 if((h->base.flags&(Dxe::UserMode|Dxe::PositionIndependent))!=(Dxe::UserMode|Dxe::PositionIndependent)||(h->base.flags&~Dxe::KnownFlags))return ValidateResult::BadHeader;
 if(h->base.reserved0)return ValidateResult::ReservedNotZero;for(u64 v:h->base.reserved)if(v)return ValidateResult::ReservedNotZero;for(u64 v:h->reserved2)if(v)return ValidateResult::ReservedNotZero;
 if(h->capabilities&~KnownCapabilities)return ValidateResult::BadCapabilities;
 if(!h->segmentCount||h->segmentCount>MaxSegments||h->segmentEntryBytes!=sizeof(Segment))return ValidateResult::BadSegmentTable;
 u64 tableBytes=(u64)h->segmentCount*sizeof(Segment);if(!range(h->segmentTableOffset,tableBytes,bytes)||h->segmentTableOffset<h->base.headerBytes)return ValidateResult::BadSegmentTable;
 if(h->relocationCount||h->relocationTableOffset)return ValidateResult::UnsupportedRelocations;
 if(h->importCount>MaxImports)return ValidateResult::BadImportTable;
 if(h->importCount){u64 ib=(u64)h->importCount*sizeof(Import);if(!h->importTableOffset||!range(h->importTableOffset,ib,bytes)||h->importTableOffset<h->base.headerBytes)return ValidateResult::BadImportTable;}
 else if(h->importTableOffset)return ValidateResult::BadImportTable;
 const Segment*s=(const Segment*)((const u8*)img+h->segmentTableOffset);bool entry=false;
 for(u32 i=0;i<h->segmentCount;i++){
  const auto&x=s[i];if(x.reserved||!x.memoryBytes||x.fileBytes>x.memoryBytes||!range(x.fileOffset,x.fileBytes,bytes))return ValidateResult::BadSegment;
  if(x.fileBytes&&x.fileOffset<h->base.headerBytes+tableBytes)return ValidateResult::BadSegment;
  if((x.virtualAddress&4095)||!x.alignment||x.alignment>4096||(x.alignment&(x.alignment-1)))return ValidateResult::BadSegment;
  if(!(x.flags&Read)||(x.flags&~(Read|Write|Execute)))return ValidateResult::BadSegment;if((x.flags&Write)&&(x.flags&Execute))return ValidateResult::WritableExecutable;
  u64 ve=0;if(!add(x.virtualAddress,x.memoryBytes,ve))return ValidateResult::ArithmeticOverflow;
  for(u32 j=0;j<i;j++)if(ov(x.virtualAddress,x.memoryBytes,s[j].virtualAddress,s[j].memoryBytes))return ValidateResult::OverlappingVirtual;
  if((x.flags&Execute)&&h->base.entryOffset>=x.virtualAddress&&h->base.entryOffset<ve)entry=true;
 }
 if(h->importCount){const Import*im=(const Import*)((const u8*)img+h->importTableOffset);for(u32 i=0;i<h->importCount;i++){const auto&x=im[i];if(x.reserved||!x.symbol||x.abiMajor!=1||x.abiMinor>0)return ValidateResult::BadImport;bool writable=false;for(u32 j=0;j<h->segmentCount;j++){u64 e=0;add(s[j].virtualAddress,s[j].memoryBytes,e);if((s[j].flags&Write)&&x.patchVirtualAddress>=s[j].virtualAddress&&x.patchVirtualAddress<=e-8){writable=true;break;}}if(!writable)return ValidateResult::BadImport;}}
 if(!entry)return ValidateResult::EntryNotExecutable;u64 st=h->base.requiredStackBytes?h->base.requiredStackBytes:65536;if(st<Dxe::MinStackBytes||st>Dxe::MaxStackBytes)return ValidateResult::BadHeader;
 if(out)*out=h;return ValidateResult::Ok;
}
const char* ResultName(ValidateResult r){switch(r){case ValidateResult::Ok:return"Ok";case ValidateResult::NotDxe2:return"NotDxe2";case ValidateResult::BadHeader:return"BadHeader";case ValidateResult::BadAbi:return"BadAbi";case ValidateResult::BadCapabilities:return"BadCapabilities";case ValidateResult::BadSegmentTable:return"BadSegmentTable";case ValidateResult::BadSegment:return"BadSegment";case ValidateResult::OverlappingVirtual:return"OverlappingVirtual";case ValidateResult::WritableExecutable:return"WritableExecutable";case ValidateResult::EntryNotExecutable:return"EntryNotExecutable";case ValidateResult::BadImportTable:return"BadImportTable";case ValidateResult::BadImport:return"BadImport";case ValidateResult::UnsupportedRelocations:return"UnsupportedRelocations";case ValidateResult::ReservedNotZero:return"ReservedNotZero";default:return"ArithmeticOverflow";}}
}
