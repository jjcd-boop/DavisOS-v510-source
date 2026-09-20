# Davis OS v4.10 — Forge Semantic Test Gate

Forge now performs a semantic-requirements validation after FBC3 compilation and structural sandbox validation, but before isolated live promotion. Intent requirements for lists/tables, search/filtering, arithmetic, text workspaces, counters, and conditional behavior must be represented by corresponding validated FBC3 operations. A semantically incomplete generation is not promoted, packaged, or installed.

Each generation also writes a versioned `_HISTORY.txt` snapshot containing the original intent, generated FDL3 plan, generation number, and semantic test result. This establishes a concrete revision trail for later diagnose/repair work.

Pipeline: intent -> compositional plan -> FDL3 -> FBC3 -> structural sandbox -> semantic requirement gate -> isolated Ring-3 runner ACK -> package -> install.

Security remains unchanged: generated FBC3 does not receive driver I/O, DMA, IRQ, raw PCI, or native machine-code authority.
