# Davis OS v21.00 — Forge Atomic Multi-Change System Plans

v21 adds a host/offline System Plan Builder above the v20 audited transformation library.

A plan contains 2–8 authenticated `.SYSSTAGE` transactions. Each transaction keeps its own component/risk/source allowlist. The builder verifies every transaction, synthesizes each bounded transformation against a disposable source workspace, rejects cross-boundary targets, applies changes in declared order, and performs one canonical Davis OS build only after all transformations have staged successfully.

The plan is atomic from Forge's perspective: there is one candidate and no partial promotion path. Any integrity, synthesis, boundary, patch, or build failure rejects the complete plan. The known-good source tree is copied and never edited in place. Promotion remains absent and requires future explicit human approval plus physical boot validation.

Artifacts include per-transaction patches, `COMBINED_PROPOSAL.patch`, `VALIDATION.json`, candidate kernel/ELF, and a candidate-only marker.
