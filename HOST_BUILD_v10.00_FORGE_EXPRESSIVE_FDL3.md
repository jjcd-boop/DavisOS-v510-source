# Davis OS v10.00 — Forge Expressive FDL3

This pass expands Forge's bounded FDL3/FBC3 application vocabulary while retaining isolated Ring-3 execution and differential regression protection.

Added primitives:
- SELECT / dropdown-style choice control
- NUMBER / numeric entry control
- PROGRESS / bounded progress presentation
- TABS / tabbed view selector
- STATUS / state indicator

Forge's intent planner can synthesize these primitives from matching requests. The compiler emits dedicated bytecodes (31–35), ForgeRunner validates and renders them, and runtime telemetry gives each primitive an exact contract bit. SELECT, NUMBER and TABS additionally carry interaction-route contract bits. Targeted evolution can repair a missing exact primitive rather than satisfying the request with an unrelated rich-UI element.

Existing protections remain: FBC3 budget validation, semantic/behavior/scenario gates, linked MODEL/VIEW/EVENTS/TESTS modules, isolated ForgeRunner execution, up to three targeted repair rounds, and candidate-versus-BEST differential regression comparison.

Limitations: this remains a bounded deterministic language, not arbitrary native-code synthesis. PROGRESS and STATUS are currently presentation/state primitives rather than general data-bound widgets. Hardware behavior requires physical Davis OS validation.
