# Davis OS v1.92 — Hardware Bring-Up

This revision targets external USB HID, audio routing, and network hardware identification.

## USB/xHCI

- Replaced the single-first-connected-port assumption with per-port xHCI enumeration.
- All connected root-hub ports present during boot are now candidates for Address Device + descriptor/configuration processing.
- Multiple boot-HID endpoints can now be armed and serviced instead of polling only the first HID slot.
- Added connected-port, HID-device, mouse, and keyboard counters to the input diagnostics overlay.
- Corrected HID SET_PROTOCOL to use the USB class/interface request type (0x21).
- Configure Endpoint now includes the Slot Context in the Add Context flags alongside the HID endpoint.
- Existing touchpad/I2C and PS/2 paths were not removed.

This specifically addresses a structural reason an external USB mouse could remain invisible even while another input device worked: v1.91 enumerated only the first connected xHCI root port and PollHid selected only the first HID-ready slot.

Current limitation: this is boot-time multi-port enumeration. Full hot-plug/change-event enumeration is still a later step.

## Audio/HDA

- Playback-route selection now prefers an internal speaker pin, then headphones, then line-out.
- Pins explicitly marked as not physically connected are skipped.
- This avoids selecting the first enumerated output pin, which on laptops may correspond to HDMI/dock or another non-speaker route.
- Audio popup now distinguishes codec/controller readiness from playback-route and DMA readiness.

## Network

- Network popup now exposes the detected Ethernet PCI vendor/device ID in hexadecimal.
- This makes unsupported hardware immediately identifiable for the next device-specific driver backend.
- The v1.90 NetworkAdapter abstraction and E1000 backend remain intact.

## Validation

`BUILD_ALL_KERNEL_LINUX.sh` completed successfully with RC=0 after the changes.

## Next physical-boot observations

1. Open input diagnostics and note USB PORTS, HID, MICE, KEYBOARDS, EVENTS, and REPORTS.
2. Move/click the external USB mouse and check whether REPORTS increments and MICE is nonzero.
3. Open Network and record the Ethernet PCI ID shown.
4. Open Audio and press TEST; note whether the popup reports ROUTE + DMA READY, ROUTE FOUND / DMA NOT READY, CODEC READY / NO PLAYBACK ROUTE, or AUDIO NOT READY.
