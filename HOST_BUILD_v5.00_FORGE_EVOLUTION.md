# Davis OS v5.00 — Forge Evolution Gate

Forge now retains a per-project BEST generation record and refuses to replace an installed program with a lower-quality candidate. Candidate programs pass structural sandboxing, semantic requirements, behavior contracts, bounded scenario validation, isolated Ring-3 execution, and a regression gate before package installation. A rejected candidate leaves the prior installed package untouched. BEST records contain bounded FBC3, generation, quality, capabilities, and integrity digest.

This is deterministic bounded program evolution, not unrestricted autonomous native-code generation. Generated applications remain FBC3 under ForgeRunner with Window + IPC authority only.
