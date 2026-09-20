# Davis OS v1.78 — Ring-3 xHCI DMA Rings

This revision advances the Ring-3 USB backend from passive xHCI capability discovery to construction of the controller's DMA data structures inside the driver process.

Implemented:
- xHCI capability validation in Ring 3.
- Ring-3 DMA allocation for a 4 KiB command ring.
- Link TRB construction with toggle-cycle semantics.
- Ring-3 DMA allocation for a 4 KiB event ring.
- Event Ring Segment Table allocation and first ERST entry construction.
- Device Context Base Address Array allocation.
- Existing PCI BAR mapping and MSI/IRQ endpoint setup retained.

Safety boundary:
- v1.78 does NOT program CRCR, DCBAAP, ERSTSZ/ERSTBA/ERDP, USBCMD, or port registers.
- The Ring-0 compatibility xHCI backend remains the active controller owner during this hardware-test boundary.
- This avoids two independent drivers mutating the same controller concurrently.

Next cutover:
- explicit controller ownership handoff;
- stop compatibility backend;
- install Ring-3 DCBAA/command/event rings;
- reset/start controller;
- enumerate ports/slots;
- control transfers and boot HID interrupt transfers;
- remove corresponding Ring-0 xHCI ownership after hardware validation.
