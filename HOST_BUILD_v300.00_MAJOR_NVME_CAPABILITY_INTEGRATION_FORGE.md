# DavisOS v300.00 — NVMe Capability Integration + Forge

## Milestone
v300 closes another controller-facing gap by making the concrete NVMe controller transport decode and enforce the controller CAP register before queue configuration or MMIO queue binding.

## NVMe capability contract
- Decodes MQES and rejects queue depths the physical controller cannot support.
- Decodes CSS and requires the NVM command set.
- Decodes DSTRD and derives the physical doorbell stride rather than accepting a disconnected hard-coded value.
- Decodes MPSMIN/MPSMAX and rejects the DavisOS 4 KiB page size when unsupported.
- `NvmeMmioTransport::BindFromController` now consumes the validated controller state and CAP-derived doorbell stride.

## Regression
`nvme_capability_integration_regression` performs 100,000 controller-to-MMIO bindings across multiple legal DSTRD values and verifies fail-closed rejection for unsupported command sets, MQES overflow, and incompatible page size.

Expected result:
`PASS nvme_capability_integration cycles=100000 css_reject=1 mqes_reject=1 mps_reject=1`

## Forge
Forge now has a bounded `kernel.nvme_controller_ready_poll_limit` transformation. Accepted range: 10,000..50,000,000. Out-of-range requests fail closed.

## Qualification
125/125 host gates pass. Physical hardware remains required before release qualification.
