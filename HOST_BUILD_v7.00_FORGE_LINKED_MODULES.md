# Davis OS v7.00 — Forge Linked Modules

Forge v7.00 turns the v6 module artifacts into a fail-closed linked module graph.

## Changes
- MODEL, VIEW, and EVENTS modules now contain their actual role-specific FDL3 lines instead of placeholder metadata.
- Every module line carries an internal sequence identity during generation.
- The linker reconstructs the exact canonical FDL3 stream from the separated modules.
- Forge recompiles the linked stream and byte-compares its FBC3 output with the already validated candidate.
- A module-link mismatch fails promotion and prevents package installation.
- TESTS modules now persist explicit bounded contracts for window presence, termination, intent telemetry, interactive state transitions, and regression protection.
- Existing v6.50 precise hit testing and installed-runner lifecycle cleanup are retained.

## Security
No native generated machine code was enabled. Generated programs remain FBC3 in isolated Ring-3 ForgeRunner processes. No driver, PCI, DMA, or IRQ authority is granted.

## Validation
Forge.dxe rebuilt with app ID 48, version 7.0, capabilities 0x17. Canonical BUILD_DAVIS_OS.sh completed successfully. Physical hardware execution remains to be verified.
