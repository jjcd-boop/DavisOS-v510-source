Davis OS v22.00 — Forge Dependency-Aware System Planner

New host tool: Tools/system-builder/system_dependency_planner.py

It can turn a bounded high-level system goal into an unauthorised plan proposal, then later consume authenticated SYSSTAGE nodes, verify their source boundaries, construct a dependency graph, topologically order the changes, and hand the ordered graph to the atomic System Plan Builder.

Forge still cannot overwrite the known-good OS. Candidate promotion remains unavailable and physical boot validation remains required.
