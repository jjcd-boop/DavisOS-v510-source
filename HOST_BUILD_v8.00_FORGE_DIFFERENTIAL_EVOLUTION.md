# Davis OS v8.00 — Forge Differential Evolution

Forge 8.00 adds same-contract differential validation between a new candidate and the retained BEST generation.

## Changes
- Candidate runtime telemetry is scored only against the current project's explicit contract mask.
- The retained BEST FBC3 program is executed through a fresh isolated ForgeRunner using the same contract mask.
- A candidate cannot replace BEST if it satisfies fewer current contracts than the retained implementation.
- Candidate telemetry is restored after baseline execution so packaging decisions refer to the candidate.
- ForgeRunner test invocations exit immediately after publishing their report; generated installed apps launched through the package handoff remain interactive.
- Kernel ForgeRunQuery now consumes completed report slots, preventing the fixed report table from filling indefinitely.
- TESTS modules declare SAME_CONTRACT_DIFFERENTIAL_NOT_WORSE.

## Security
No native generated code. FBC3 remains bounded and interpreted in a Ring-3 SystemService ForgeRunner with Window + IPC only. No PCI, DMA, IRQ, or driver authority is delegated.

## Validation limitation
Host compilation/build success does not prove physical-machine GUI or hardware behavior. Test on the Davis OS target.
