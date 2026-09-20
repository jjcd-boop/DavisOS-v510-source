# Davis OS v1.90 — Stability + Network Abstraction

This pass continues the v1.89 stabilization work without changing the known-working touchpad path.

## Changes

1. Added `NetworkAdapter`, a hardware-neutral NIC boundary between the IPv4/DHCP/ARP/DNS/TCP stack and concrete Ethernet drivers.
   - The network service no longer calls E1000 TX/RX directly.
   - Current backend selection recognizes the implemented Intel E1000-family driver.
   - Unsupported Ethernet controllers are retained as detected hardware rather than being confused with "no controller".
   - The desktop network popup now reports the selected/unsupported backend state.

2. Hardened E1000 initialization cleanup.
   - If DMA rings/buffers are allocated but MAC-register validation fails, all four DMA allocations are released before initialization fails.
   - This closes a boot-time physical-memory leak on partially compatible or unexpectedly initialized Intel controllers.

3. Preserved the v1.89 64 MiB kernel mapping contract and 64-bit PCI BAR decoding.

## Why this matters

The protocol stack is now independent of the E1000 implementation. Adding another wired NIC family (for example Realtek or a newer Intel family) no longer requires rewriting DHCP, ARP, DNS, ICMP, TCP, or browser networking. A backend only needs initialization, send, receive, link state, and MAC handoff.

## Validation

`BUILD_ALL_KERNEL_LINUX.sh` completes successfully after the changes. The existing kernel build/test gates remain intact.

## Next hardware pass

Boot this image on the target system and read the Network popup. If Ethernet is detected but unsupported, the PCI vendor/device pair already captured by `NetworkService::state` identifies the exact backend that should be implemented next. Application launch diagnostics remain the next stability target: distinguish a missing window from a faulted/exited/not-yet-scheduled Ring-3 process at the shell boundary.
