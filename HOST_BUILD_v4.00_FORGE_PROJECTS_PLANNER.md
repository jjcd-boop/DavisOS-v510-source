# Davis OS v4.00 — Forge Projects + Compositional Planner

This release moves Forge from one-shot generation toward a persistent development environment.

## Added
- Versioned `.PROJECT` records with FNV-1a integrity validation.
- Save and Reopen controls inside Forge.
- Restores project name, intent, and generation counter.
- Compositional intent planner recognizes finance, list/table, math, note/text, counter, filter/search, and conditional/threshold features.
- Planner can combine modifiers (for example searchable table tracker or threshold-aware counter) rather than selecting only one fixed archetype.
- Bounded deterministic repair fallback: a failed FDL3 compile is replaced with a minimal safe utility plan and compiled/tested again before promotion.
- Existing compile -> sandbox -> isolated Ring-3 ACK -> package -> install pipeline remains fail-closed.

## Security
Project files are authoring data only. Generated execution remains FBC3 interpreted in isolated ForgeRunner processes. No driver I/O, DMA, IRQ, raw PCI, or arbitrary native code is granted.

## Limits
This is not a general natural-language AI planner. Planning is deterministic and bounded. Automatic repair currently repairs invalid generated plans, not arbitrary semantic bugs.
