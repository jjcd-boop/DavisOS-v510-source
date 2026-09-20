# DavisOS v370.00 — AP Pre-Paging Integrity Seal + Forge Protection

## Major change
The AP trampoline now authenticates every handoff field it consumes before the normal C++ validator can execute. Handoff ABI v2 adds a 32-bit assembly-verifiable pre-paging integrity seal over the first 64 bytes: magic, ABI, APIC/logical identity, CR3, stack top, kernel entry point, rendezvous token, and generation.

The .code32 trampoline verifies the seal before loading CR3. It also retains the low-CR3/alignment checks. Therefore corruption of stackTop, entryPoint, rendezvousToken, or generation cannot silently survive until long mode and redirect the AP before the higher-level checksum validator runs.

## Qualification
- 100,000 generated handoffs
- 500,000 deliberate critical-field/seal corruptions rejected
- ABI v2, 76-byte packed handoff
- executable trampoline remains relocation-free
- Forge explicitly rejects attempts to disable/change the pre-paging seal/checksum contract
- Full release gate: 150/150 PASS

## Hardware status
Host qualification only. Physical AP execution remains required before release qualification.
