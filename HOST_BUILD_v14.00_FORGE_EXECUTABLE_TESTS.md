# Davis OS v14.00 - Forge Executable Tests

Forge v14 promotes the TESTS module from documentation-only assertions to a compact executable test contract carried with each isolated validation run.

## Changes
- RunReq now carries a bounded executable test mask and remains below the Davis IPC payload limit.
- Forge derives tests from the current intent contract: window boot, Enter mutation, pointer action, focus, multistep transactions, state coherence, checkbox toggle, select advance, and tab switching.
- ForgeRunner executes the requested test contract against the validated FBC3 event/control graph before reporting PASS.
- A failed requested test clears the normal-termination proof, causing Forge telemetry validation to fail closed.
- TESTS .fmod now records explicit TEST declarations in addition to ASSERT contracts.
- Existing role-scoped MODEL/VIEW/EVENTS evolution, retained-BEST differential gate, Ring-3 isolation, and capability restrictions remain in place.

## Scope
The test harness is deterministic and bounded. It tests the FBC3 transition model; it does not inject physical keyboard/mouse events into the desktop. Physical target testing is still required.
