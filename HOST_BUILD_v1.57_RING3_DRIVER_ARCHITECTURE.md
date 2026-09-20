# Davis OS v1.57 — Ring-3 Driver Architecture Transition

This revision establishes the new privilege contract: Ring 0 is kernel-only; drivers, services, desktop and applications are Ring 3. Rings 1/2 are unused.

Implemented now:
- explicit SecurityDomain model (Kernel/SystemService/Driver/Application)
- CPU-ring contract: kernel=0, every non-kernel domain=3
- DXE2 application loader defaults every ordinary executable to Application
- executable-requested capabilities are filtered through kernel SecurityPolicy
- privileged driver/service capabilities are reserved and cannot be self-granted by an ordinary DXE2 application
- build gates enforce application-domain default and policy-filtered capability grants
- existing per-process page-table/UserMemory validation and W^X user mappings remain in force
- v1.48 Input.cpp and tested PS/2 controller implementation remain unchanged

Migration truth:
Existing hardware driver implementations are still compatibility code in the kernel image. This revision does NOT falsely claim they execute at CPL3 yet. Each USB/audio/network/storage/input/graphics hardware implementation must be converted to a user-space driver executable and launched by a trusted driver manager before it is counted as isolated.

Next migration sequence:
1. trusted Ring-3 driver manager + kernel-created Driver domain
2. kernel-mediated device resource handles (MMIO/PIO/IRQ)
3. migrate audio
4. migrate network
5. migrate USB
6. migrate storage
7. migrate input only after the new transport path is hardware-proven
8. graphics hardware interface
9. IOMMU/DMA confinement
