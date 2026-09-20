# DavisOS v40.00 — Major Reliability Milestone

This release intentionally groups multiple engineering passes into one milestone.

## Runtime changes
- NetworkCore now validates IPv4 header checksums on receive.
- Fragmented IPv4 UDP datagrams are rejected until reassembly is implemented, preventing partial/ambiguous delivery.
- UDP/IPv4 parsing has stricter IHL and length validation.
- ServiceSupervisor now gives critical services a degraded grace interval, then enters bounded restart/backoff instead of remaining degraded indefinitely.
- Existing v31 terminated-process slot recycling remains enabled.

## New executable host qualification
- 10,000-packet UDP build/parse stress loop, corruption rejection, fragmentation rejection, and frame bounds.
- Service timeout, degraded-state, restart/backoff, heartbeat recovery, and quarantine testing.
- Storage object bounded-write/read-integrity, process revocation, and maintenance testing.
- Existing scheduler, IPC, memory, process lifecycle, security, isolation, Forge fail-closed, artifact, and canonical-build gates remain mandatory.

## Qualification
35/35 host gates pass.

VM boot remains NOT RUN because QEMU is unavailable in the execution environment. Physical hardware validation remains required before release.
