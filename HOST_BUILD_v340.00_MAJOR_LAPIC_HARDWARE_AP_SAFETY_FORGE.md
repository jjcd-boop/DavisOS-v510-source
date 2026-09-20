# Davis OS v340.00 — LAPIC hardware delivery + AP transition safety + Forge

## Major changes
- Added `Kernel/LapicHardware.*`, a concrete LAPIC IPI transport for xAPIC MMIO and x2APIC MSR delivery.
- Hardware delivery now waits for ICR delivery-status idle before and after each write and fails closed on bounded timeout.
- xAPIC writes destination-high before ICR-low; x2APIC emits the complete 64-bit ICR through MSR 0x830.
- Added 100,000-plan stress coverage: 400,000 INIT/SIPI steps across xAPIC and x2APIC plus stuck-delivery timeout rejection.
- Fixed AP trampoline transition safety: the current 16→32→64-bit bootstrap loads CR3 in 32-bit protected mode, so low-memory image construction now rejects CR3 values above 4 GiB instead of silently truncating them.
- Forge gained bounded `kernel.lapic_delivery_poll_limit` tuning (1,000..10,000,000).

## Qualification
- 139 / 139 host qualification gates pass.
- Canonical freestanding kernel build passes.
- Physical AP execution remains a required hardware qualification step before release.
