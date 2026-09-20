# Davis OS v2.01 — Forge Live Ring-3 Promotion

Forge now promotes validated FBC1 programs into a separate preloaded Ring-3 execution host named ForgeRunner.

## Implemented
- Added `ForgeRunner.dxe` (`AppForgeRunner=49`) as a kernel-trusted **SystemService domain** process. It still executes at x86 CPL3 in its own address space.
- Added Davis system service ID 4 (`ForgeRunner`). ForgeRunner registers an IPC receive endpoint through the existing service registry.
- Forge performs: intent capture -> source emission -> manifest -> FBC1 compile -> local bounded validation -> IPC live promotion.
- The promoted FBC1 image is revalidated by ForgeRunner before execution. Unknown opcodes, oversized programs, invalid budgets/capabilities, and malformed one-window programs are rejected.
- ForgeRunner receives only Window + IPC capabilities. It has no driver I/O, DMA, IRQ, raw PCI, or arbitrary executable-memory capability.
- Validated generated programs appear immediately in a separate Davis window hosted by ForgeRunner without rebooting.
- Manifest records `LIVE RING3 RUNNER: PASS/FAIL`.

## Security / honesty boundary
This is live execution in a separate Ring-3 address space, but FBC1 remains interpreted bytecode. Forge does **not** yet emit native x86-64 DXE machine code at runtime, and ForgeRunner is a reusable isolated execution host rather than a freshly allocated process for every generation. Native dynamic DXE image creation/loading remains a later milestone.
