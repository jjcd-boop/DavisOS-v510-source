# Davis OS v1.51 — Safe Boot + Hardware Probe

Adds loader-controlled SAFE_BOOT.TXT and HARDWARE_PROBE.TXT modes, plus default physical-storage write protection. HARDWARE_PROBE performs read-only PCI/ACPI enumeration and halts. SAFE_BOOT skips storage, xHCI, HDA, network activation and DXE auto-launch while retaining the core desktop path.

Physical disk writes remain opt-in only via DAVIS/ALLOW_PHYSICAL_WRITES.TXT. Current BlockDevice physical backends are not yet exposed for general writes; this flag establishes the policy contract before that changes.

Validation: freestanding host compile/link only. No physical boot claim.
