# DavisOS v390.00 — Physical Corroboration Probe

Physical-machine follow-up to v380.

- Probe v1.30 adds read-only raw PCI config corroboration (BDF, command, BAR0/BAR1) for storage, NIC and xHCI.
- NIC identity is independently re-matched against the immutable PCI scan to catch BDF formatting/state corruption.
- MADT enabled APIC IDs are checked for duplicates before SMP activation; duplicate topology is surfaced as a warning and remains non-activating in probe mode.
- Storage remains strictly read-only/non-activating. No PCI config writes are issued by HardwareProbe.
- Host qualification: 156/156 PASS on final run. One prior run exposed the known timing-sensitive simulated NVMe end-to-end regression (rc=3); rerun passed. This is not represented as physical NVMe proof.
- Physical hardware proof remains required for promotion.
