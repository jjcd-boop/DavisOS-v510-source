# Davis OS v12.00 — Forge Lineage Evolution

Forge v12 extends v11 competitive evolution into three bounded evolutionary epochs per Generate operation.

Each epoch evaluates up to three FDL3 candidates in isolated ForgeRunner processes. The winning candidate becomes the seed for the next epoch. Selection prioritizes runtime contract satisfaction and uses smaller FBC3 bytecode only as a tie-breaker. The final epoch winner must still pass module-link validation, telemetry validation, and the retained-BEST differential regression gate before installation.

A generation-specific `_LINEAGE.txt` artifact records completed epochs, candidates exercised, final winning variant, and contract score. This is bounded deterministic evolution; it is not unrestricted self-modifying native code. Generated programs remain FBC3 executed in isolated Ring-3 ForgeRunner processes without raw PCI, DMA, IRQ, or driver authority.

Physical hardware behavior remains to be validated on Davis OS hardware.
