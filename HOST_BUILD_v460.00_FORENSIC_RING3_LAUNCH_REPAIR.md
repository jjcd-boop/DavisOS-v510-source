# DavisOS v460.00 — Forensic Ring-3 Launch Repair

This milestone applies the v450 forensic-audit findings rather than extending the v450 regression blindly.

## Changes
- Removed eager production `BootAppRunner::RunGeneric()` execution from the kernel boot path. Boot now prepares Ring-3 applications but does not execute the entire cohort before the desktop loop.
- Added explicit bounded `BootAppRunner::LaunchKind()` first-user-slice execution from an actual desktop launch request.
- Desktop now launches a prepared process on demand, then immediately retries owner-window discovery.
- Added an immutable boot application manifest snapshot with an integrity seal, independent of mutable runtime application/process state.
- Expanded diagnostics APPS footer with BOOT/APPS/SEAL counts.
- Fixed canonical build provenance: `BUILD_DAVIS_OS.sh` now emits `out/BOOTX64.EFI`, satisfying the qualification artifact gate.
- Retained v450 supervisor-only xAPIC MMIO mapping; this revision does not revert Ring-0/Ring-3 isolation.

## Qualification
162/162 PASS — QUALIFIED_HOST.
Physical hardware remains required before release/promotion.
