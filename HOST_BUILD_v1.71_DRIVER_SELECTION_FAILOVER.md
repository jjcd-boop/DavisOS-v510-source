# Davis OS v1.71 — Driver Selection / Failover Foundation

Implemented from v1.70 source.

- Added `DriverSelection` runtime inventory.
- PCI devices are classified into USB, storage, audio, Ethernet, Wi-Fi, and graphics families.
- Every compatible `DriverCatalog` entry is ranked by score for each discovered device.
- Up to eight compatible candidates are retained per device rather than keeping only one match.
- `FailSelected()` advances to the next ranked candidate and marks the exhausted binding inactive.
- Driver candidate discovery runs after syscall/object/resource-broker initialization and before desktop startup.
- This is selection/failover infrastructure. It does not falsely claim that every catalog backend has a complete hardware implementation.
- Existing Ring-3 driver DXEs remain staged for Audio, Graphics, Input, Network, Storage, USB, and Wi-Fi.
- Physical legacy hardware algorithms still present in Ring 0 remain migration debt; v1.71 does not claim their removal.
- Writer and Calculator/Math Lab launch routes were audited: desktop tiles and Programs menu call `FocusApp(AppWriter/AppCalculator)`, Boot/Main recognizes Writer.dxe and Calculator.dxe, and both DXEs are staged.
