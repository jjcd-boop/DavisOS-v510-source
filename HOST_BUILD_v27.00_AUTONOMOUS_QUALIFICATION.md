# Davis OS v27.00 — Autonomous Qualification Foundation

v27 converts development from a collection of manual build paths into a deterministic qualification pipeline.

- `build_kernel_linux.sh` now delegates to the canonical all-kernel build, eliminating stale hand-maintained link lists.
- `Tools/qualification/qualify.py` performs release-tree, Ring-3 default-domain, capability-policy, integrity-fail-closed, desktop/service-isolation, Forge syntax, adversarial fail-closed, kernel artifact, and memory regression gates.
- Forge System Builder v27 now requires host qualification after every canonical candidate build before a candidate can be packaged.
- Qualification never promotes a candidate. Physical hardware validation remains mandatory before release/promotion.
- QEMU boot is recorded as unavailable rather than falsely claimed when the host lacks QEMU.

This is the first layer of the autonomous engineering loop: build -> qualify -> reject or retain candidate -> later boot/hardware validation. Future revisions can add VM boot telemetry, networking conformance, filesystem stress, scheduler latency, application compatibility, and performance comparison gates without weakening the fail-closed promotion model.
