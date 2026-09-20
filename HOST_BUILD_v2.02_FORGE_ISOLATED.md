# Davis OS v2.02 — Forge Per-Program Isolation

Forge promotion now requests a fresh ForgeRunner clone for each validated FBC1 generation.

## Kernel additions
- `GetPid` syscall (3).
- `SpawnIsolated` syscall (54), capability-gated by IPC and restricted to the ForgeRunner template.
- `MultiAppRuntime::SpawnIsolatedFromKind()` clones the boot-loaded ForgeRunner DXE2 image into a new process, page-table/address-space, guarded kernel stack, user stack, scheduler/preemption context, and SystemService security domain.
- Runtime application capacity expanded to 64 while boot image capacity remains unchanged.

## Private handoff
Each spawned ForgeRunner discovers its PID and registers a private service ID at `0x1000 + pid`. Forge waits for that private endpoint and sends the validated FBC1 image to that process. The original boot ForgeRunner remains only as a compatibility fallback.

## Security boundary
The spawned runner receives only the capabilities encoded in the ForgeRunner DXE2 image: Window + IPC. It receives no file, DMA, IRQ, PCI, or driver capabilities. Generated FBC1 remains interpreted and independently validated inside the fresh runner.

## Status
Host build completed. Forge and ForgeRunner rebuilt as DXE2. Kernel rebuilt. BOOTX64.EFI relinked. Physical-hardware validation remains required.
