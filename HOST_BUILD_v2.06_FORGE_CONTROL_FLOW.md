# Davis OS v2.06 — Forge Control Flow

Forge FDL2 now includes bounded named-action and control-flow primitives compiled to FBC2.

## Added language/runtime primitives
- FUNCTION declarations (`OP_FUNC`)
- CALL bindings (`OP_CALL`)
- IF/ELSE branch state (`OP_BRANCH`)
- List filter bindings (`OP_FILTER`)
- Existing typed state, events, arithmetic, comparisons and records remain supported.

Forge synthesis now composes these primitives into expense/budget, inventory/list, calculator and note utilities. ForgeRunner independently allow-list validates the new opcodes before execution and renders live branch/action/filter state.

## Security boundary
Generated programs still execute through a fresh isolated ForgeRunner Ring-3 process. ForgeRunner retains Window + IPC only (0x11). Forge retains Window + IPC + File Read/Write (0x17). No driver I/O, DMA, IRQ, raw PCI or arbitrary executable-memory capability is granted.

## Build validation
Forge.dxe and ForgeRunner.dxe compiled and packed successfully. Canonical BUILD_DAVIS_OS.sh completed successfully. All three packaged kernel binaries are byte-identical. Physical hardware validation is still required.
