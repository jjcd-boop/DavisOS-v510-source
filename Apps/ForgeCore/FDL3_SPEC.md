# FDL3 language contract
FDL3 is a bounded declarative application language. A program declares one or more views, typed state, records, widgets, named actions and event bindings.

## Types
NUMBER, BOOL, TEXT, RECORD, LIST<RECORD>, LIST<TEXT>.

## UI
WINDOW, LABEL, INPUT, NUMBER_INPUT, BUTTON, LIST, TABLE, TEXTAREA, VALUE.

## Logic
FUNCTION/CALL, SET, ADD/SUB, COMPARE, IF/ELSE, FILTER, APPEND, CLEAR, bounded FOREACH.
No recursion, arbitrary pointers, self-modifying code, native jumps, MMIO, ports, DMA or IRQ instructions are legal.

## Events
ON_CLICK, ON_ENTER, ON_CHANGE, ON_OPEN. Every handler has a static instruction budget.

## Capability declarations
WINDOW and IPC are baseline. STORAGE_PRIVATE, NETWORK_CLIENT, TIMER and NOTIFY are brokered capabilities and must be granted by Davis policy; they are not equivalent to raw file, PCI, DMA or driver authority.
