# FBC3 bytecode contract
Fixed-width four-byte instructions. Validation is mandatory before promotion and repeated by ForgeRunner.
Limits: <=128 byte executable plan in the current transport, <=128 validation steps, exactly one root window in the current runner, bounded state arrays and bounded text/list storage.
Future transport expansion must version the wire message rather than silently increasing it beyond Davis IPC payload limits.
