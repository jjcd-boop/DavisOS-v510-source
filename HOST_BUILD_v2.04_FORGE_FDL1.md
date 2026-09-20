# Davis OS v2.04 — Forge FDL1

Forge now synthesizes a constrained declarative program description (FDL1) before bytecode emission. The pipeline is intent -> FDL1 -> FBC1+ -> bounded validation -> isolated ForgeRunner process.

## FDL1 v1 surface
Current compiler recognizes WINDOW, LABEL, INPUT, LIST, COUNTER, TEXT, BUTTON, IF INPUT_NONEMPTY, CALC ADD_INPUT, and END. Generated `.forge` files are preserved beside `.cpp` and `.fbc` artifacts for inspection and revision history.

## New composition paths
Inventory/todo/list/tracker intents synthesize editable input + bounded list + conditional add behavior. Notes synthesize editable text state plus conditional commit semantics. Counter/tally requests synthesize counter state and actions. Calculator/math requests synthesize numeric input, counter state, and ADD_INPUT calculation behavior.

## Safety
FDL1 is not native machine code. The compiler emits only allow-listed FBC1+ opcodes. The validator enforces one window, bounded bytecode size, bounded execution budget, known opcodes, and capability limits. ForgeRunner remains Window + IPC only (DXE2 caps 0x11). Forge remains Window + IPC + File Read/Write (0x17) to write source, FDL1, FBC and manifests. No driver I/O, DMA, IRQ, raw PCI, arbitrary executable memory, or self-granted capabilities were added.

## Build
Forge.dxe and ForgeRunner.dxe were rebuilt and copied into USB_ROOT/DAVIS/APPS. The canonical BUILD_DAVIS_OS.sh completed successfully. Kernel source was unchanged by this revision; the packaged kernel copies remain byte-identical. Physical hardware validation is still required.
