# Davis OS v28.00 — Forge Autonomous Hardening

v28 strengthens the autonomous build/qualification loop and expands Forge's bounded performance-tuning vocabulary.

- Forge System Builder and dependency planner now expose `kernel.scheduler_quantum` as an audited transformation. Values are restricted to 1..64 ticks; unsafe values fail closed.
- Qualification now self-tests the new Forge transformation and its rejection boundary.
- Qualification compiles both Forge and qualification Python tooling, checks Ring-3 driver source trees for direct privileged-I/O helper calls, verifies the UEFI PE artifact, verifies the kernel ELF magic, and retains all v27 security/isolation/build/memory gates.
- The canonical kernel is rebuilt from all Kernel C++/assembly sources and must pass the hardware-preservation input gates before qualification can succeed.
- Candidate promotion remains deliberately separate from candidate generation. Forge can create and qualify candidates but cannot silently overwrite the known-good generation.

Host qualification result for this source generation: 23/23 PASS. QEMU is still unavailable in this sandbox, so VM boot remains explicitly unclaimed and physical-hardware validation remains required.
