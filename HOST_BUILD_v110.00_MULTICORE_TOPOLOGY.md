# Davis OS v110.00 — Multicore Topology Foundation

This major autonomous pass adds a fail-closed ACPI MADT parser as groundwork for SMP and IOAPIC interrupt routing.

## Runtime changes
- Added `Kernel/AcpiMadt.*`.
- Discovers enabled Local APIC and x2APIC processor entries.
- Discovers IOAPIC controllers and their GSI bases.
- Discovers ISA interrupt-source overrides and resolves legacy IRQs to GSIs.
- Honors a valid Local APIC address override when it fits the current 32-bit MMIO model.
- Rejects malformed/truncated MADT records rather than trusting partial firmware topology.
- Bounded topology inventories prevent firmware tables from overrunning kernel storage.

## Qualification
The host release gate now contains 59 cumulative checks. The v110 native regression builds against the real ACPI table and MADT implementations and validates a synthetic two-CPU topology, IOAPIC discovery, IRQ override routing, and malformed-entry rejection.

Result: 59/59 PASS in the available host sandbox.

VM boot remains NOT_RUN_NO_QEMU_AVAILABLE. Physical hardware qualification is still required before release.
