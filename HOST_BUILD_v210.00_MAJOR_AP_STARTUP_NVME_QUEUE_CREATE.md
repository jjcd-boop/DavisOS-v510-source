# Davis OS v210.00 — AP Startup + NVMe Queue Creation

This milestone adds two hardware-facing contracts while preserving fail-closed host qualification.

## SMP/AP startup
- Added a validated x86 INIT/deassert/SIPI/SIPI startup plan.
- AP trampoline must be 4 KiB aligned and below 1 MiB.
- Added xAPIC/x2APIC ICR encoders and bounded startup-attempt configuration.
- Host regression exhaustively exercises all 255 legal SIPI vectors across 64 APIC IDs.
- This does **not** claim secondary processors have executed Davis kernel code yet; privileged LAPIC delivery and AP trampoline execution still require physical/VM validation.

## NVMe controller queue creation
- Added Create I/O Completion Queue and Create I/O Submission Queue command construction.
- Validates queue ID, queue depth, 4 KiB physical alignment, interrupt vector, CQ/SQ association and physical-contiguous flags.
- 100,000 queue-creation plans are stress-validated.
- This does **not** claim physical NVMe I/O; admin-command submission against real controller hardware remains required.

## Forge
- Forge can tune SMP AP startup attempts only in the audited 1..5 range.
- Unsafe values fail closed.

## Qualification
- Cumulative host qualification: 91/91 PASS.
- Physical hardware validation remains mandatory before release promotion.
