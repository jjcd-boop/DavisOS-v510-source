# Davis OS v1.75 - Ring-3 DMA Handoff

Adds capability-gated contiguous DMA allocation/free for Driver-domain DXE2 processes.
DMA pages are mapped writable/NX only into the requesting driver's address space and are reclaimed on process teardown.
The syscall returns both user virtual and physical bus address under the current identity-mapped standalone memory contract.
Allocation is bounded to 16 MiB per request and alignment to <=2 MiB power-of-two.

This is not IOMMU isolation: devices can still DMA to arbitrary physical memory if malicious/misprogrammed. IOMMU setup remains required before calling DMA strongly isolated.
MSI/MSI-X programming and a general external-interrupt dispatch path are not yet complete. Legacy IRQ binding exists but is not claimed as full interrupt delivery.
No Ring-0 xHCI/HDA/storage/network compatibility implementation is removed in this revision.
