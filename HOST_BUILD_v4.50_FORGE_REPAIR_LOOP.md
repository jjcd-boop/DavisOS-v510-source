# Davis OS v4.50 — Forge Repair Loop

Forge now performs a bounded diagnose/repair/retest cycle before live promotion.

Pipeline: intent -> FDL3 -> FBC3 -> structural sandbox -> semantic requirements -> bounded repair -> behavior contract -> isolated Ring-3 ACK -> package -> install.

Repairs are capability-minimal and limited to four passes. Missing window/list/table/filter/arithmetic/text/counter/conditional semantics are diagnosed by fault code and patched into the FDL3 plan, then recompiled and revalidated. A behavior contract rejects inert interactive plans and inconsistent branch/filter plans. A 0-100 generation quality score records validation depth and penalizes repaired plans slightly.

Security boundary is unchanged: generated applications remain validated FBC3 hosted by ForgeRunner with Window + IPC only. No raw PCI, DMA, IRQ, driver I/O, or native-code emission is introduced.
