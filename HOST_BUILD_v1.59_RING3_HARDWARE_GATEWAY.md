# Davis OS v1.59 — Ring-3 Hardware Gateway

This revision adds the first kernel-mediated hardware gateway intended for isolated Ring-3 driver processes.

Implemented:
- DriverGateway kernel component with per-process MMIO mapping records and IRQ bindings.
- DriverMapMmio syscall maps only an already-claimed MMIO resource into the owning driver's user address space, NX and user-accessible, within a dedicated 0x60000000–0x70000000 aperture.
- DriverPortRead / DriverPortWrite mediate 8/16/32-bit x86 port I/O through the kernel. Ring-3 drivers never execute IN/OUT directly.
- DriverBindIrq binds an already-claimed IRQ resource to a receive-capable IPC endpoint owned by the driver.
- DriverGateway::DeliverIrq() provides the kernel-side IRQ-to-IPC delivery hook. Interrupt-controller integration for individual hardware drivers is intentionally not switched on yet.
- Process teardown revokes gateway mapping records and IRQ bindings before broker resources are released.
- Existing resource handles, driver-domain checks, CapDriverIo/CapIrq checks, and broker ownership remain mandatory.
- Added 16-bit x86 port I/O primitives for the mediated gateway.

Not yet claimed/implemented:
- Existing USB, PS/2 input, audio, storage, network, or graphics drivers have NOT yet been migrated out of Ring 0.
- Driver MMIO mappings are not yet dynamically unmapped on resource release/process exit; records are revoked, but page-table unmap/TLB shootdown infrastructure is a later prerequisite for safe hot restart.
- IRQ delivery hook exists but legacy interrupt handlers have not been rerouted to Ring-3 drivers.
- DMA/IOMMU isolation is not yet active.
- No attack testing was performed.

Input regression rule: the known v1.48 Input.cpp and PS/2 path were not deliberately modified in this pass.
