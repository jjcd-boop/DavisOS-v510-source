# DavisOS v350.00 — SMP Platform Binding

This milestone binds trusted ACPI MADT topology and IA32_APIC_BASE/CPUID platform facts to the concrete LAPIC IPI transport. xAPIC startup now fails closed if MADT and APIC-base pages disagree or if firmware exposes a destination requiring x2APIC while the processor is not operating in x2APIC mode. x2APIC mode supports wide APIC IDs through the MSR ICR path.

Host qualification stresses 100,000 complete platform bindings and INIT/SIPI plans across xAPIC and x2APIC, plus negative tests for wide-ID/xAPIC mismatch, LAPIC-base disagreement, and unavailable APIC hardware.

Physical AP execution remains a required target-machine qualification item.
