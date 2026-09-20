# Davis OS v1.74 - Ring-3 PCI Resource Handoff

- Added PciResources: supported PCI functions publish BAR MMIO/I/O resources and legacy IRQ-line metadata into the kernel-owned DeviceResourceBroker.
- BAR sizing is performed with PCI decode temporarily disabled and original BAR/command values restored.
- Added privileged DriverResourceInfo syscall (73). It only exposes resources belonging to the kernel-assigned device of the calling Ring-3 Driver domain.
- All seven Ring-3 driver processes now enumerate their assigned hardware resources through syscall 73 rather than relying only on a synthetic identity.
- Existing claim/map/PIO/IRQ capability checks remain in force; enumeration does not grant ownership.
- No DMA window is fabricated. DMA allocation/IOMMU confinement and MSI/MSI-X delivery still require implementation before physical xHCI/HDA/Wi-Fi/storage cutover can be called complete.
- Legacy IRQ line metadata is published only when firmware exposes a nonzero/non-0xFF line; modern MSI/MSI-X is not falsely represented as implemented.
- Existing kernel compatibility hardware algorithms remain active in this build to preserve boot/input while the Ring-3 transport becomes capable enough to replace them.
