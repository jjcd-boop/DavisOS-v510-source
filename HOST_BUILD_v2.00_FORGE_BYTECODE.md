# Davis OS v2.00 — Forge FBC1 Compiler and Sandbox

Forge now has a real constrained compilation stage. It continues to emit revisioned Davis C++ source, and additionally compiles the requested safe archetype into FBC1 bytecode (`.fbc`). A bounded validation VM executes the bytecode before the generation is marked valid.

## Security boundary
FBC1 is deliberately non-privileged. Its instruction set can describe a window, labels, buttons, counter/text state, and declared persistence. The validator has an instruction budget, rejects unknown opcodes, enforces one-window structure, and does not expose driver I/O, IRQ, DMA, raw PCI, kernel memory, or arbitrary native instructions.

## Honest limitation
This revision does **not** yet promote FBC1 into a dynamically loaded DXE2 native executable. `DXE PROMOTION` remains pending. That requires a kernel-mediated dynamic image loader and lifecycle API rather than giving Forge direct process-table or executable-memory authority.
