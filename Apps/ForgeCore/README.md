# Forge 3 Core
Forge 3 is split into five contracts so the foundry can grow without coupling generated programs to kernel authority.

- `FDL3_SPEC.md`: declarative source language contract.
- `FBC3_SPEC.md`: bounded bytecode/runtime contract.
- `PACKAGE_SPEC.md`: generated application package and manifest contract.
- `SECURITY_MODEL.md`: capability and isolation contract.
- `TEST_PLAN.md`: compiler/runtime validation matrix.

The current executable implements the safe core of these contracts. Features that require new Davis brokers (persistent private storage, networking, timers/notifications, package installation) are specified here but are not falsely exposed to generated programs until those brokers exist.
