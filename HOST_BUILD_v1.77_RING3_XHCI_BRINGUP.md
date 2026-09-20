# Davis OS v1.77 - Ring-3 xHCI Bring-up

- UsbDriver now claims and maps its assigned xHCI MMIO BAR through DriverGateway.
- Ring-3 code parses xHCI capability registers (CAPLENGTH, HCIVERSION, HCSPARAMS, HCCPARAMS, DBOFF, RTSOFF) and validates offsets.
- MSI endpoint binding and DMA allocation remain active in the Ring-3 USB driver.
- Safety rule: v1.77 does not write xHCI operational registers while the Ring-0 compatibility USB backend still owns live hardware. This avoids two controllers racing the same device during first hardware validation.
- This is the first controller-specific Ring-3 hardware bring-up code, but not yet a complete xHCI cutover. Reset/run, command/event rings, slot enumeration, control transfers and HID interrupt transfers remain in the compatibility backend pending the next cutover.
