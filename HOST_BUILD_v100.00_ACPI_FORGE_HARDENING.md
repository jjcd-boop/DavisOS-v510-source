# DavisOS v100.00 — ACPI + Forge Hardening

Major autonomous milestone.

- ACPI subordinate SDTs now fail closed on checksum failure rather than entering the trusted table inventory.
- Added native ACPI regression covering RSDP extended checksum, XSDT traversal, FADT/DSDT discovery, and corrupt SSDT rejection.
- Expanded Forge qualification to continuously verify bounded audio-stream and USB-poll transformations, including unsafe-value rejection.
- Full canonical kernel build and cumulative host qualification pass.
- VM boot remains untested in this environment because QEMU is unavailable; physical hardware qualification remains required.
