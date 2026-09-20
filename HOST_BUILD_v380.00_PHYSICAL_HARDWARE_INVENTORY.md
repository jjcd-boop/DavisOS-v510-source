# DavisOS v380.00 — Physical Hardware Inventory

Physical v370 testing proved the read-only UEFI/PCI probe executes on the target machine and revealed that no native NVMe/AHCI class function is directly exposed. v380 turns that observation into a targeted, non-destructive inventory pass.

## Changes
- Physical probe v1.20 prints ACPI MADT CPU counts, enabled CPUs, IOAPIC count, LAPIC base, and the first APIC IDs.
- Mass-storage PCI functions are printed before generic device summaries, preventing storage-controller identity from being hidden by display row limits.
- PCI state now tracks all class 0x01 mass-storage functions, RAID/remapping class functions, and a conservative Intel storage-remapping/VMD candidate classification.
- VMD detection is classification-only. It never binds, enables, bus-masters, or writes the candidate controller.
- Network, xHCI, audio, and display BDF/vendor/device/class triples are promoted into the physical photo output.
- Probe remains read-only and halts for photography.

## Qualification
153/153 host qualification gates pass. Canonical freestanding kernel build passes.

Physical storage, SMP AP execution, NIC operation, external USB mouse, and audio remain unproven until target-machine testing.
