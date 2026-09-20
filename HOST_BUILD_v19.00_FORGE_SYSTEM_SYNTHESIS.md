# Davis OS v19.00 — Forge Bounded System Synthesis

v19 extends the offline System Builder so an authenticated `.SYSSTAGE` request can produce a source patch without a separately supplied diff, but only through explicit audited synthesis recipes. Unknown requests fail closed.

## New path

`SYSSTAGE -> verify -> synthesize bounded patch -> source allowlist -> disposable workspace -> canonical build -> candidate + validation`

`--synthesize` and `--patch` are mutually exclusive. Every synthesized proposal is saved as `PROPOSED.patch`, hashed in `VALIDATION.json`, and subjected to the same component boundary checks as externally supplied patches.

The first recipe is intentionally narrow: a desktop-scoped request can rename the visible workspace label in `Kernel/Desktop.cpp`. This proves request-to-source construction without granting a generic source-writing primitive. Unsupported kernel, driver, bootloader, or ambiguous OS requests are rejected rather than guessed.

No promotion mechanism exists. Known-good source is never edited in place. Physical boot validation and explicit approval remain future gates.
