# Davis OS v6.00 — Forge Adaptive Modules

Forge v6.00 extends the verified FDL3/FBC3 pipeline with four project module artifacts (MODEL, VIEW, EVENTS, TESTS), richer bounded UI declarations, and a targeted runtime-telemetry repair pass.

## New bounded UI operations
- LAYOUT
- PANEL
- CHECKBOX
- DIALOG
- SCROLL

ForgeRunner independently accepts and renders these operations and reports a dedicated rich-UI telemetry bit. Requests for dashboards, forms, settings, panels, toggles, dialogs, or scrolling now require that telemetry before promotion.

## Targeted repair
After a structurally/semantically valid candidate launches, Forge compares actual ForgeRunner telemetry with the intent-derived telemetry contract. If a required runtime behavior is missing, Forge patches only the missing FDL3 feature class, recompiles, reruns the sandbox/scenario validators, launches a fresh isolated runner, and checks telemetry again. Promotion remains fail-closed.

## Modules
Each successful generation emits bounded `.fmod` artifacts for MODEL, VIEW, EVENTS, and TESTS. These are project decomposition artifacts; FBC3 remains the executable representation.

## Security
No native code generation was added. Generated programs remain FBC3 interpreted in isolated Ring-3 ForgeRunner processes. ForgeRunner capabilities remain Window + IPC (0x11); Forge remains 0x17. No generated driver, DMA, IRQ, or raw PCI authority.

## Build validation
Forge and ForgeRunner rebuilt as DXE2 app IDs 48 and 49. Canonical BUILD_DAVIS_OS.sh completed successfully.
