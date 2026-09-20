# DavisOS v310.00 — NVMe Storage Runtime Integration + Forge

This milestone closes a major integration gap between NVMe identification and usable I/O queue creation.

## Added
- `Kernel/NvmeStorageRuntime.hpp/.cpp`
- Allocates page-aligned DMA memory for an NVMe I/O submission queue and completion queue.
- Binds the controller's real admin DMA queues to the CAP-derived MMIO doorbell transport.
- Builds Create I/O CQ/SQ admin commands and executes them through Admin Queue 0.
- Preserves admin queue head/tail/phase state across the transition.
- Frees I/O queue DMA resources on failure and shutdown.
- Rejects queue depths beyond controller MQES / DavisOS bounds.
- StorageService now advances from Identify completion into I/O queue bring-up instead of stopping after namespace discovery.
- Forge's existing bounded `kernel.nvme_io_queue_depth` transformation now directly constrains the operational storage-runtime default as well as the queue implementation ceiling.

## Regression
`PASS nvme_storage_runtime admin_db=4 io_depth=64 dma_release=1 oversize_reject=1`

The host test uses a concurrent fake controller to consume the real admin SQ and publish real-format CQ entries. It verifies four Admin Queue 0 doorbell writes, successful queue creation, DMA teardown, and oversized-depth rejection.

## Qualification
127/127 host qualification gates passed. Physical hardware qualification remains required before release.
