# DavisOS v400.00 - Physical Desktop Boot

v400 switches the physical USB from stop-after-inventory mode to the normal DavisOS boot path.
The v390 read-only inventory established the target machine's PCI and MADT identity; v400 therefore removes
HARDWARE_PROBE.TXT from the USB payload while retaining physical-storage read-only default behavior.

Boot chain under test:
UEFI -> loader checkpoints -> kernel load/BSS zero -> DXE preload -> GOP/ACPI -> memory map -> ExitBootServices
-> DavisKernelMain -> memory/runtime/platform -> USB/I2C-HID/input -> desktop -> Ring-3 cohort.

The loader banner is updated to DavisBoot Desktop v1.53. The package does not include DIAGNOSTIC_BOOT.TXT,
so successful desktop initialization is not followed by an intentional diagnostic halt.

Physical storage writes remain disabled unless ALLOW_PHYSICAL_WRITES.TXT is deliberately added; v400 does not add it.
Host canonical kernel build passes. Physical desktop success remains unproven until the USB is booted on target hardware.
