# DavisOS v180.00 — NVMe PRP/DMA Planning

This milestone adds bounded multi-page PRP planning for NVMe transfers and deterministic queue doorbell offset calculation. The PRP planner handles an unaligned first page, direct PRP2 for one additional page, and a bounded PRP list for larger transfers. Oversized mappings fail closed.

Host qualification exercises 100,000 generated mappings and multiple controller doorbell strides. This does not claim physical-controller DMA success; real queue creation, MMIO writes, interrupt completion, timeout/reset, and hardware qualification remain required.
