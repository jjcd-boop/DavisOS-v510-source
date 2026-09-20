# DavisOS v90.00 — Major Filesystem Lifecycle Milestone

This milestone hardens the in-kernel file-object layer and autonomous qualification system.

## Runtime changes
- Added strict absolute-path validation for file opens and namespace mutations.
- Rejects relative paths, dot/dot-dot traversal components, control characters, backslash path ambiguity, overlong paths, and invalid roots.
- Added bounded file seek semantics.
- Added safe rename semantics that refuse destination collisions and refuse mutation while a file is open.
- Added safe deletion semantics that refuse deletion while a file is open.
- Preserves owner isolation for open-file operations.

## Forge
- Added bounded `kernel.max_file_bytes` transformation.
- Forge accepts 4096..65536 bytes and rejects values outside that range.

## Qualification
- Canonical host qualification uses the complete all-kernel freestanding build.
- Added 5,000-cycle filesystem lifecycle stress covering create/write/seek/read/close/rename/delete and namespace/path attacks.
- Full cumulative release gate: 51/51 PASS.
- VM boot remains NOT_RUN_NO_QEMU_AVAILABLE; physical hardware qualification remains required.
