# Davis OS v1.89 — Stability + Versatility Pass 1

This pass was made from the supplied v1.88 source tree without relying on prior project memory.

## Critical Ring-3 stability correction
The loader and physical allocator both protect a 64 MiB kernel aperture at physical `0x02000000`, but `AddressSpace::KernelImageSpan` still mapped only 16 MiB into each Ring-3 process CR3. That mismatch can leave live kernel/BSS pages unmapped whenever a syscall, interrupt, scheduler transition, or trap executes under a process page table. v1.89 makes the per-process supervisor kernel contract 64 MiB as well.

This is a stronger, directly observable contract defect than the allocator-only mismatch fixed in v1.88 and is a plausible explanation for applications failing immediately after launch.

## PCI/network versatility
PCI memory BAR decoding now handles both 32-bit and 64-bit MMIO BARs. The E1000 path no longer truncates a 64-bit BAR to the legacy `bar0` field before using it. This improves compatibility on firmware/platforms that place NIC MMIO above 4 GiB.

## Network failure containment
Partial E1000 DMA allocation failures now release any rings/buffers that were already allocated rather than leaking physical pages. This makes repeated/recovery initialization safer.

## Intentionally not claimed
This does not add drivers for arbitrary Realtek/Broadcom/Intel Wi-Fi or non-E1000 Ethernet controllers. Detection and driver support remain separate. It also does not change audio or USB mouse behavior in this pass.
