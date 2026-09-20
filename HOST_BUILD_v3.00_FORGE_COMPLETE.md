# Davis OS v3.00 — Forge consolidated architecture

This release consolidates the Forge prototype into an FDL3/FBC3 foundry core and adds explicit dependency contracts for language, bytecode, packages, security and testing.

Implemented executable core: typed numeric/text/bool state opcodes, records/lists/tables, inputs/text areas, functions/calls, events, comparisons/branches, arithmetic, filters, clear/subtract primitives, bounded validation, independent ForgeRunner validation, and fresh Ring-3 process promotion.

Security remains fail-closed: live ForgeRunner is Window+IPC only. Raw file access, PCI, DMA, IRQ and driver I/O are not granted. Private persistence/network/timer/notification/package-installation are specified dependencies, not falsely advertised as live capabilities; they require dedicated Davis brokers in future kernel/service work.

This is a consolidated source implementation, not a claim that arbitrary software descriptions can already be synthesized. Physical-hardware validation remains required.
