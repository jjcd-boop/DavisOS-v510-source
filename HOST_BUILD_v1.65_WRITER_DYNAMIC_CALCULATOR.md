# Davis OS v1.65 — Dynamic Writer + Visual Calculator

## Writer
- Removed the 511-character fixed document buffer.
- Writer text and per-character rich-text metadata now grow dynamically from the kernel page allocator.
- Capacity doubles on demand; there is no fixed document character constant. Practical size is bounded only by available/allocatable memory and system resource policy.
- Cursor and selection positions are now 64-bit.
- Existing rich-text, templates, review, thesaurus, and insert-object UI retained.

## Calculator / Math Lab
- Added a fully visible calculator keypad.
- Added persistent RESULT display.
- `=` button and Enter evaluate expressions.
- Supports integer +, -, *, / and parentheses with division-by-zero/error reporting.
- Traditional mode uses standard integer arithmetic.
- Howard experimental mode preserves the explicitly defined 1*1=2 rule; no unspecified Howard rules are invented.
- Keyboard entry remains supported alongside pointer input.

## Preserved baseline
- Input.cpp and Ps2Diagnostics.cpp were not modified.
