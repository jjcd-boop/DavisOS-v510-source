# Davis OS v2.05 — Forge FDL2

Forge now synthesizes FDL2 and compiles it to constrained FBC2 bytecode before validation and isolated Ring-3 execution.

## FDL2 additions
- Named typed state declarations (`VAR NUMBER`, `VAR TEXT`).
- Multiple independent input fields (up to three bounded fields in the current runner).
- Event bindings for Enter and click actions.
- Arithmetic bindings and comparison/branch state.
- Structured record declarations for small record-oriented utilities.
- Existing window, label, list, counter, text and button primitives remain available.

Forge has new composition paths for inventory/todo tools, calculators, notes, counters, and expense/budget utilities. Budget/record programs can combine amount + description fields, a record model, arithmetic total state and comparison state.

## Runtime and safety
ForgeRunner v2.05 accepts only FBC2 magic and allow-listed opcodes. Validation caps bytecode at 128 bytes and execution budget at 128 operations. Generated applications remain interpreted in a fresh isolated ForgeRunner Ring-3 process. ForgeRunner retains Window + IPC only (0x11). Forge retains Window + IPC + File Read/Write (0x17). No driver I/O, DMA, IRQ, raw PCI or arbitrary executable memory was granted.

## Build
Forge.dxe and ForgeRunner.dxe were rebuilt and copied to USB_ROOT/DAVIS/APPS. Canonical BUILD_DAVIS_OS.sh completed successfully. Kernel source did not require modification for FDL2; all packaged kernel copies are byte-identical. Physical-hardware validation remains required.
