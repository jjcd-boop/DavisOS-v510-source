# Davis OS v1.93 Hardware Recovery

Hardware-tested v1.92 feedback drove this pass.

## Application boot image recovery
UEFI DXE discovery no longer relies only on firmware directory enumeration. After enumeration, the loader explicitly opens every known Davis DXE filename and fills any missing BootInfo entries. This directly targets `APP: NOT PRESENT IN BOOT IMAGE` and `DXE PROOF NOT READY` observed on hardware while preserving directory discovery for future generic DXEs.

## Realtek RTL8168/RTL8111 Ethernet
Added a polling-first RTL8168-family backend and connected it to NetworkAdapter. The hardware-reported PCI ID `10EC:8168` now selects `REALTEK RTL8168/8111` rather than UnsupportedEthernet. The bring-up path enables PCI MMIO/bus mastering, resets the controller, creates DMA TX/RX descriptor rings, reads the MAC, enables RX/TX, and connects send/receive to the existing Davis network stack. Interrupts are intentionally disabled for the first hardware validation; polling reduces variables during bring-up.

## USB mouse recovery
The kernel xHCI/HID runtime no longer relinquishes the controller merely because a Ring-3 USB driver requests ownership. The previous handoff could set `ready=false` before the Ring-3 driver proved that HID transfers were operational, stranding external USB input. v1.93 keeps the known kernel xHCI path authoritative until a transactional prove-before-commit ownership protocol is implemented.

## Wi-Fi diagnostics
The Network popup now exposes the Wi-Fi PCI vendor/device ID so the next hardware boot identifies the exact wireless chipset driver required.

## Validation
`BUILD_ALL_KERNEL_LINUX.sh` completes successfully after the changes. `BOOTX64.EFI` was rebuilt from the revised UEFI loader, and both USB kernel copies were replaced with the v1.93 kernel.
