# Davis OS v290.00 — NVMe Admin Doorbell Correctness

This milestone corrects a controller-facing NVMe defect discovered during integration review.

Create-I/O-CQ and Create-I/O-SQ are NVMe *admin commands*. They are submitted through Admin Submission Queue 0 and their completions are consumed from Admin Completion Queue 0. Earlier NvmeAdminRuntime code passed the I/O queue identifiers to the transport when ringing doorbells. With the concrete MMIO transport bound to QID 0, those writes were rejected, meaning host fake-transport tests could pass while a real controller would never be notified.

v290 changes the runtime to ring SQ0 for both create commands and CQ0 after consuming each completion. A new regression composes the real NvmeAdminRuntime with the real NvmeMmioTransport and verifies:

- two commands are published to Admin SQ memory;
- SQ0 doorbell at 0x1000 advances to 2;
- CQ0 doorbell at 0x1004 advances to 2 for 4-byte stride;
- I/O queue 1 doorbells remain untouched during queue creation;
- command order is Create I/O CQ (0x05), then Create I/O SQ (0x01);
- four total MMIO doorbell writes occur (two SQ, two CQ).

Stress result:

`PASS nvme_admin_doorbell admin_sq=2 admin_cq=2 doorbells=4 io_db_untouched=1`

The existing 100,000-cycle admin-runtime stress test remains active and now expects the corrected four-doorbell lifecycle.

Hardware qualification remains required. This host test proves the address/routing semantics against emulated MMIO memory; it does not claim a physical NVMe controller accepted the commands.
