# DavisOS v250.00 — NVMe MMIO Transport + Recovery + Forge

This milestone adds a concrete MMIO-facing NVMe queue transport with submission/completion queue indices, phase handling, SQ/CQ doorbell writes, memory barriers, and controller-disable reset signaling. It also adds bounded quiesce/reset/reinitialize recovery and a Forge recipe for recovery retry policy.

Host qualification uses emulated MMIO memory and queue buffers. This does NOT claim successful physical-controller DMA, MSI/MSI-X delivery, or physical NVMe persistence. Those require hardware qualification.
