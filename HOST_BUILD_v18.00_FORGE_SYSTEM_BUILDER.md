# Davis OS v18.00 — Forge Offline System Builder

v18 adds `Tools/system-builder/system_builder.py`, a host/offline consumer for Forge `.SYSSTAGE` transactions.

## Implemented gates
- verifies FSTG magic, supported version, request digest, policy digest, flags, and transaction digest;
- refuses unscoped whole-OS transactions;
- maps the transaction component to an explicit source allowlist;
- parses proposed unified-diff targets and rejects traversal/cross-component writes;
- copies the known-good source tree into a separate workspace;
- applies the patch only to that copy;
- records before/after SHA-256 tree digests;
- invokes the canonical `BUILD_DAVIS_OS.sh` in the candidate workspace;
- copies candidate kernel artifacts separately and writes `VALIDATION.json`;
- contains no promotion/overwrite operation.

## Trust boundary
Forge remains Ring 3 on the Davis target. The System Builder is a host/offline development tool because the target image does not contain the full compiler/source environment. A successful build is still only a candidate. Physical boot validation, rollback readiness, and explicit human approval remain required before any future promotion mechanism.
