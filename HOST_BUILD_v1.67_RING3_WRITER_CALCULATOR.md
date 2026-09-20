# Davis OS v1.67 — Writer/Calculator Ring-3 Extraction

## Purpose
Remove Writer and Calculator execution/document/evaluator logic from the kernel before further hardware testing.

## Implemented
- Removed Writer document buffers, dynamic document allocator, formatting state, review/thesaurus/template logic, editing logic, and Writer rendering from `Kernel/Desktop.*`.
- Removed Calculator/Math expression state, parser/evaluator, result state, and native calculator rendering from `Kernel/Desktop.*`.
- Writer and Calculator are now independent DXE2 application processes in Ring 3.
- Both obtain compositor windows through capability-controlled IPC and present user-owned pixel buffers through the window syscall.
- DesktopService now routes keyboard events to the focused DXE application without changing the tested Input/Ps2 transport implementation.
- Added user-process page allocation/free syscalls so Writer document storage can grow geometrically without a fixed character-count limit. Allocation remains bounded by available memory/address space.
- Writer rendering only traverses the visible tail of the document. A fault in Writer terminates/contains the user process rather than executing Writer logic in Ring 0.
- Calculator is a visual Ring-3 app with pointer/keyboard entry and a dedicated RESULT display. Supports integer + - * / and parentheses with precedence and division-by-zero error handling.
- Native shell no longer opens the old kernel Writer/Math windows. Legacy WindowManager numeric IDs remain reserved for ABI/layout stability but contain no Writer/Calculator implementation.

## Important migration limitation
The v1.67 Ring-3 Writer is intentionally a reduced editor for the isolation cutover: dynamic text entry/backspace/newlines and visual document display are functional, but the v1.64/v1.65 rich formatting/templates/review/thesaurus/object-insertion features have not yet been reimplemented inside the Ring-3 app. They were removed from Ring 0 rather than retained as kernel compatibility code. Restore those features in Ring 3 only.

## Protected input baseline
`Kernel/Input.cpp` SHA-256: abe4bcb8ff6dcbd921cf05d0bcc1c43b1a5e02f119f9bf063b23d90f8162f890
`Kernel/Ps2Diagnostics.cpp` SHA-256: cb596a0d7eeffc739fd4efea3c7339874014153ef478febbbab4c92059159e3c
Neither file changed.
