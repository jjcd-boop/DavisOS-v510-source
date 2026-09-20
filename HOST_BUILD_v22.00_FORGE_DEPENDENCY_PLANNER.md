# Davis OS v22.00 — Forge Dependency-Aware System Planner

v22 adds `Tools/system-builder/system_dependency_planner.py` above the v21 atomic builder.

## High-level planning
`--goal` conservatively decomposes a request only when each clause maps to exactly one audited v20 transformation. It emits `PLAN_PROPOSAL.json`; it does not fabricate authorization. Each node must later be represented by an authenticated `.SYSSTAGE` transaction.

## Authorized execution
For 2–8 authenticated stages the planner:
1. verifies every SYSSTAGE through the existing integrity/policy verifier;
2. proves each request still maps to an audited transformation and allowlisted source target;
3. builds a dependency graph;
4. orders Ring-0 foundations before Ring-3 driver/desktop dependents and boot changes after runtime components;
5. serializes same-file changes deterministically;
6. topologically sorts the graph and rejects cycles/duplicates;
7. passes the ordered plan to the v21 all-or-nothing System Plan Builder.

The known-good source remains immutable. There is no automatic promotion path.

## Host validation performed
A three-node goal (kernel audio streams, Ring-3 USB timeout, boot GOP label) decomposed into three proposal nodes with kernel -> driver -> boot dependencies. Three authenticated test SYSSTAGE records were deliberately supplied in reverse order; v22 reordered them kernel -> driver -> boot and the atomic builder accepted the ordered plan. A full canonical build was then exercised through the planner.

Physical boot validation remains required before any candidate can be considered for promotion.
