# Forge 3 validation plan
Compiler: reject unknown directives, overflow, missing END, invalid type/action references and bytecode overflow.
Validator: reject wrong magic/version, malformed length, excessive budget, unknown opcode, multiple root windows and undeclared capability bits.
Runtime: exercise counter, calculator, note, inventory and record applications; keyboard focus; button hit testing; close/relaunch; process isolation; malformed IPC payload rejection.
Security: generated program must fail raw file/PCI/DMA/IRQ/driver operations and must not inherit Forge authoring capabilities.
Hardware: final acceptance requires physical Davis OS boot and interaction testing.
