#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
mkdir -p USB_ROOT/EFI/BOOT USB_ROOT/DAVIS/APPS
cp -f EFI/BOOT/BOOTX64.EFI USB_ROOT/EFI/BOOT/BOOTX64.EFI
cp -f DAVISKRN.BIN USB_ROOT/DAVIS/DAVISKRN.BIN
find Apps -name '*.dxe' -size -2M -exec cp -f {} USB_ROOT/DAVIS/APPS/ \;
echo "USB_ROOT ready. Copy it to a FAT32 thumb drive."
