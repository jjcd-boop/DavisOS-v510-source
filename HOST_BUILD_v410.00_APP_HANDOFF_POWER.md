# DavisOS v410.00 — Deterministic App Handoff + Power

Physical v400 proved UEFI -> kernel -> desktop on target hardware, but desktop launches reported APP: NOT PRESENT IN BOOT IMAGE.

v410 removes firmware directory enumeration as the authoritative application ordering. The UEFI loader now explicitly stages the known application manifest in deterministic application-first order before optional Ring-3 drivers. This prevents a driver preparation failure from starving desktop applications merely because firmware returned directory entries in a different order.

Power support adds a desktop Start-menu SHUT DOWN command. The kernel retains the UEFI Runtime Services pointer and invokes ResetSystem(EfiResetShutdown) after disabling interrupts, then fails closed to HaltForever if firmware unexpectedly returns.

Physical storage remains read-only by default.

Host qualification: 156/156 PASS on the complete rerun. One preceding run reproduced the known timing-sensitive synthetic NVMe end-to-end test failure (155/156); no storage code was changed by v410.
