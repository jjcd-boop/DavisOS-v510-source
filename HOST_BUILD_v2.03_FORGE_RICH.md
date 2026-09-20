# Davis OS v2.03 — Forge Rich Generated Applications

Forge FBC1 was extended with stateful application primitives: editable input, bounded list models, semantic actions, and keyboard event handling. Inventory/todo/list/tracker intents now compile to an interactive input + list application. Note/text intents compile to an editable workspace with an action. Each promoted generation still runs in a fresh ForgeRunner Ring-3 process/address space.

## FBC1 additions
- OP_INPUT: bounded editable text state.
- OP_LIST: bounded six-entry list model.
- OP_ACTION: semantic generated-program action.
- Sandbox validator recognizes and counts the new state cells and controls.
- ForgeRunner independently validates the expanded instruction set.

## Runtime behavior
- Keyboard events update generated application state.
- Enter commits input into bounded list models.
- Pointer interaction can commit list input.
- All generated state is bounded; no arbitrary pointers or native execution are exposed.

## Security
ForgeRunner remains Window + IPC only (DXE2 caps 0x11). No file, driver I/O, DMA, IRQ, or raw PCI capability was added to the live generated-program host. Forge itself retains Window + IPC + File Read/Write (0x17) for source/manifest/FBC artifacts.

## Build status
Forge and ForgeRunner compiled and converted to DXE2. Canonical Davis kernel build passed. Kernel copies in the USB tree are byte-identical. BOOTX64.EFI was not relinked because the bootloader source was unchanged in this revision. Physical-hardware validation remains required.
