# Davis OS v320.00 — End-to-End NVMe DMA + Forge

## Major milestone
v320 closes the production storage data-path gap between BlockDevice and the v310 NVMe storage runtime.

- StorageService now binds the live NVMe I/O runtime into BlockDevice after successful controller/namespace/I/O-queue bring-up.
- NvmeBlockBackend now has a production runtime binding that uses the real MMIO submission/completion queue transport and rings the I/O SQ/CQ doorbells.
- Arbitrary kernel/user buffers are no longer assumed DMA-addressable. Each active transfer receives allocator-owned DMA bounce storage; writes copy into DMA memory before submission and reads copy back only after a successful completion.
- Multi-page transfers build PRP plans and use a physically-addressed PRP list when required.
- DMA bounce and PRP-list allocations are released on completion, submission failure, cancellation, timeout, and unbind.
- Completion polling is now CID-independent at the MMIO CQ head, avoiding head-of-line deadlock when completions arrive for a different active BlockDevice request.
- NVMe Flush command construction was corrected: Flush no longer incorrectly requires a nonzero block count.
- MMIO transport queue depth validation now permits the larger of the configured admin/I/O queue limits rather than accidentally imposing the admin limit on I/O queues.
- Forge adds a bounded `kernel.nvme_max_transfer_bytes` recipe: 4 KiB through 1 MiB, 4-KiB multiples only.

## End-to-end host stress
The integrated test drives BlockDevice -> bounce DMA -> NVMe command -> MMIO SQ doorbell -> simulated controller -> CQ -> CQ doorbell -> BlockDevice completion.

`PASS nvme_end_to_end cycles=2000 io=4021 bounced=4000 sqdb=4021 cqdb=4021 flush=1`

The test performs 2,000 write/read data-integrity cycles with transfers from 1 to 64 sectors (up to 32 KiB), including multi-page PRP-list traffic and periodic Flush commands.

## Qualification
`QUALIFIED_HOST 131/131`

Physical NVMe hardware execution remains required before release qualification.
