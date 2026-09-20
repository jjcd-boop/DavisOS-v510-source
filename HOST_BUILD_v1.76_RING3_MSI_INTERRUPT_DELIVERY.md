# Davis OS v1.76 — Ring-3 MSI Interrupt Delivery

Implemented in this revision:
- PCI MSI capability discovery for supported PCI devices.
- MSI resources published through the existing device-resource broker.
- Sixteen kernel-owned external interrupt vectors (0x50–0x5F) installed in the IDT.
- New capability-gated DriverConfigureMsi syscall (76).
- MSI programming is only allowed after the assigned Ring-3 driver exclusively claims the MSI resource.
- External MSI interrupts are converted into bounded IPC messages through DriverGateway::DeliverIrq and acknowledged through the APIC controller.
- MSI is disabled again during driver process teardown.
- All seven Ring-3 driver DXEs now discover MSI resources, create a receive endpoint, claim/configure MSI, and bind the interrupt resource to that endpoint when available.
- Existing BAR/MMIO/PIO and DMA handoff remains in place.

Truth/limitations:
- MSI-X is not implemented in this revision.
- IOMMU DMA confinement is not implemented yet.
- Legacy INTx routing through IOAPIC is not implemented by this new path.
- The driver DXEs now have a usable interrupt-delivery substrate, but device-specific xHCI/HDA/NVMe/AHCI/Wi-Fi algorithms have not yet all been physically removed from Ring 0.
- Wi-Fi firmware loading, 802.11 association, WPA2/WPA3, and chipset-specific command engines remain pending.
- No claim is made that every device supports MSI; unsupported devices remain on compatibility/fallback paths.
