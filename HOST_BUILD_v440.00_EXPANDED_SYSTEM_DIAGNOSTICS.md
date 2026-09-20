# DavisOS v440.00 — Expanded System Diagnostics

Physical v430 evidence showed app click requests reaching prepared Ring-3 processes without subsequent dispatch/entry/window events.

v440 expands the in-desktop diagnostics console from a 96-event raw list to a 256-event bounded ring and two views:
- APPS: every prepared process with app kind, PID, process state, preparation stage, ready flag, launch count, last fault vector, plus scheduler current PID/ticks/switches.
- EVENTS: sequence, event kind, PID, and four payload fields. Launch clicks now snapshot process state, app stage, ready state, and scheduler current PID.

The console remains heap-free/bounded. Pause and Clear remain available. The page control switches APPS/EVENTS.

Physical storage writes remain disabled. Host qualification: 156/156 QUALIFIED_HOST.
