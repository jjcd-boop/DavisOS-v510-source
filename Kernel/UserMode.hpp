#pragma once
#include "../Base/Types.hpp"
namespace Davis::UserMode {
struct SegmentSelectors { u16 kernelCode,kernelData,userData,userCode,tss; };
struct Tss64 { u32 reserved0; u64 rsp0,rsp1,rsp2,reserved1,ist1,ist2,ist3,ist4,ist5,ist6,ist7,reserved2; u16 reserved3,iomapBase; } __attribute__((packed));
struct GdtState { u64 entries[7]; Tss64 tss; SegmentSelectors selectors; bool ready; };
void BuildGdt(GdtState&, u64 kernelStackTop);
bool Validate(const GdtState&);
// Hardware activation is intentionally separate; v0.56 does not jump to Ring 3 yet.
void LoadGdtAndTss(const GdtState&);
}
