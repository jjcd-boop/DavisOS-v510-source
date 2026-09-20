# Davis Forge System Builder v21

`system_builder.py` verifies one authenticated SYSSTAGE transaction and creates an isolated candidate using the audited bounded transformation library.

`system_plan_builder.py` accepts a JSON plan containing 2–8 SYSSTAGE paths. It verifies each transaction independently, enforces each component allowlist, applies all transformations to one disposable workspace, then runs one canonical build. Any failure rejects the complete plan. There is no promotion operation.

v20/v21 audited transformations include desktop workspace label, desktop Applications label, kernel audio stream limit, Ring-3 USB polling timeout, and boot GOP diagnostic label. Unsupported requests fail closed.
