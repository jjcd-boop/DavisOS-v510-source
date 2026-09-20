# Davis OS v280.00 — Integrated SMP Bring-up + Forge

This milestone integrates the previously separate AP topology, bootstrap, authenticated handoff, executable low-memory trampoline image, INIT/SIPI plan, LAPIC transport, per-CPU runtime and rendezvous concepts behind a BSP-side SMP bring-up coordinator.

## Added
- `Kernel/SmpBringup.*`: bounded BSP-to-AP startup orchestration.
- `Kernel/ApRendezvous.*`: release/acquire authenticated per-CPU rendezvous mailbox.
- Complete per-AP sequence: handoff build -> low-memory image install -> topology StartupPending -> INIT/SIPI delivery -> rendezvous observation -> bootstrap transition -> runtime/topology online.
- Fail-closed behavior for low-memory install failure, IPI failure, rendezvous timeout and identity/token/generation mismatch.
- Forge bounded recipe `kernel.smp_rendezvous_poll_limit` (1,000..10,000,000).

## Host qualification
- 10,000 four-CPU system bring-up cycles.
- 29,896 successful AP online transitions.
- 104 deliberately injected rendezvous failures, all contained.
- Full release gate: 119/119 PASS.

## Hardware boundary
The orchestration and trampoline are target-architecture code, but host qualification does not prove a physical AP reached the kernel entry. Real-machine validation remains required.
