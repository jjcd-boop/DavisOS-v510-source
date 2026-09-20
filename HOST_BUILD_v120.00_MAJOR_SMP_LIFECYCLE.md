# DavisOS v120.00 — Major SMP Lifecycle Milestone

This generation converts v110 MADT discovery into a bounded kernel SMP topology/lifecycle model. Enabled processors are admitted from trusted MADT data, the BSP is explicitly identified, duplicate APIC identities fail closed, AP startup has a three-attempt ceiling, and processors transition through discovered/pending/online/failed states. A host regression performs 10,000 four-CPU topology/startup cycles and verifies bounded retry behavior.

This is groundwork for real INIT/SIPI AP startup and per-CPU scheduler state; it does not claim secondary processors have yet executed kernel code on physical hardware.
