# Davis OS v1.57 security architecture contract

## CPU privilege
- Ring 0: Davis kernel only.
- Ring 3: drivers, system services, desktop/compositor, and applications.
- Rings 1 and 2 are intentionally unused.

## Ring-3 trust domains
Ring number does not imply equal authority. Every Ring-3 process has a separate address space and a kernel-assigned domain: Driver, SystemService, or Application. Executable metadata is never sufficient to promote a process into Driver or SystemService.

## Driver target model
A driver is a Ring-3 process with only device-specific capabilities. The kernel mediates MMIO/PIO, IRQ delivery, DMA mappings and IPC. IOMMU confinement is required before untrusted DMA is considered contained.

## Migration rule
Existing in-kernel hardware implementations are compatibility code until individually migrated. No source file may be described as isolated merely because a Driver domain exists. Each migrated driver must execute at CPL3 in a user page-table context before it is counted as Ring-3 isolated.

## Application rule
Applications are Ring 3 and cannot request driver capabilities for themselves. Capability grants are the intersection of executable requests and kernel policy.
