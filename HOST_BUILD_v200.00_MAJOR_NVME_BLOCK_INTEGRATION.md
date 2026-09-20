# DavisOS v200.00 — NVMe / BlockDevice Integration + Timeout Recovery + Forge

## Milestone
This release connects the NVMe I/O queue engine to the generic asynchronous BlockDevice layer through a dedicated NVMe backend. Filesystem/storage callers can now target an NVMe-backed BlockDevice without knowing NVMe command details.

## Added
- `Kernel/NvmeBlockBackend.*`: BlockDevice-to-NVMe adapter.
- Read/write/flush translation into NVMe commands and SQ publication.
- Completion propagation from NVMe CQ back into BlockDevice request status.
- DMA mapping contract with fail-closed rejection when a buffer cannot be mapped.
- Controller error propagation into generic storage failures.
- NVMe command cancellation on BlockDevice timeout so timed-out requests cannot permanently consume queue slots.
- `Nvme::CancelIo()` bounded queue reclamation primitive.
- BlockDevice backend cancellation callback.
- Forge bounded transformation for BlockDevice request queue capacity (8..128).

## Native regression
`nvme_block_backend_regression` performs 20,000 BlockDevice->NVMe request cycles, injected controller failure, invalid DMA mapping rejection, and 100 repeated timeout/cancel/reap cycles. It verifies the NVMe queue returns to zero outstanding commands after timeout torture.

## Qualification
85/85 host qualification gates pass. Canonical all-kernel freestanding build passes.

## Hardware boundary
This release validates the integrated software path but does not claim physical NVMe controller I/O. Real controller queue creation, MMIO doorbell writes against mapped hardware, MSI/MSI-X completions, IOMMU/DMA mapping, and hardware reset recovery remain required.
