# Davis OS v9.00 — Forge Contract-Directed Evolution

Forge v9.00 replaces the previous aggregate rich-UI telemetry with exact bounded contracts for layout, panel, checkbox, dialog, and scroll primitives. Checkbox, dialog, and scroll also receive separate interaction-route telemetry. Targeted repair now inserts the exact missing primitive and can repeat diagnose -> patch -> rebuild -> isolated run -> retest for up to three bounded rounds before the differential regression gate.

Generated FBC3 remains isolated in Ring-3 ForgeRunner with Window + IPC only; no raw PCI, DMA, IRQ, or driver authority is granted. Dialog/scroll telemetry proves a bounded runtime route, not physical desktop input injection.
