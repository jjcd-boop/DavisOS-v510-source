# DavisOS v360.00 — AP Pre-Paging Trust Boundary + Forge

## Major change
The executable SIPI trampoline now validates the AP handoff's magic, ABI version, byte size, CR3 high dword, non-zero CR3, and 4 KiB CR3 alignment while still in 32-bit protected mode and before loading CR3/enabling long-mode paging.

This closes a trust-boundary defect where the C++ handoff validator ran only after the assembly trampoline had already consumed CR3. A corrupt low-memory handoff could therefore influence paging before the authenticated kernel entry had a chance to reject it.

## ABI hardening
`ApTrampoline.hpp` now publishes and statically checks the pre-paging ABI offsets used by assembly. `ValidatePrePaging()` mirrors the assembly-safe subset of the contract. `ApLowMemory::Build()` requires both the full handoff checksum validation and the pre-paging contract.

The assembled trampoline is also qualification-gated as relocation-free before it may be copied to an arbitrary legal SIPI page below 1 MiB.

## Forge
Forge now explicitly refuses transformations that attempt to modify the AP handoff magic/version/offset contract or weaken the pre-paging CR3-width contract. These are treated as hardware trust-boundary invariants, not tunables.

## Stress
`PASS ap_trampoline_prepaging cycles=100000 magic=1 abi=1 cr3_32bit=1 alignment=1`

ELF relocation audit: `There are no relocations in this file.`

## Qualification
147/147 PASS. Canonical freestanding kernel build PASS.

Kernel SHA-256: `73b0b3fff63e1dd75ce158dc7541442514fb3b107a7a78db65f27b95ea2ce127`
Kernel size: 405816 bytes.

## Remaining hardware proof
Host qualification does not prove a physical AP executes the SIPI trampoline. Physical multicore rendezvous remains required before claiming SMP hardware success.
