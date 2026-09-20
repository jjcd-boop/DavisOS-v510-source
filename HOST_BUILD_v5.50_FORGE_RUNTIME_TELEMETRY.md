# Davis OS v5.50 — Forge Runtime Telemetry

Forge v5.50 adds an execution-feedback gate to the existing bounded evolution pipeline.

ForgeRunner now performs a bounded, side-effect-free runtime probe over accepted FBC3 before reporting successful activation. The report records observed window, input, event, mutable-state, collection, arithmetic, branch/comparison, action/call, END-reachability, and simulated event-to-state-mutation behavior.

Forge derives required telemetry from the project intent and refuses promotion when the isolated runner does not report the required behavior. Runtime telemetry therefore participates in quality scoring and the regression gate. A candidate that fails telemetry cannot replace the retained best generation.

This remains constrained FBC3 execution. It does not enable arbitrary native machine code, driver I/O, PCI, DMA, or IRQ authority.

Build validation: Forge and ForgeRunner DXE2 rebuilt successfully; canonical BUILD_DAVIS_OS.sh passed.
