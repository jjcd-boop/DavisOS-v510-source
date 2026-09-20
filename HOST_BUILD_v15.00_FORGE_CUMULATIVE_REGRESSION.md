# Davis OS Forge v15.00 - Cumulative Regression Baseline

Forge v15.00 preserves executable project tests across successful generations.

## Changes
- Adds a persistent per-project `<name>_FORGE.TESTBASE` record.
- Current intent-derived executable tests are ORed with all retained promoted tests.
- Candidate variants, repair rounds, final winner, and retained-BEST differential runs execute the cumulative test mask.
- The test baseline advances only after package installation and BEST-generation promotion both succeed.
- Failed/rejected candidates cannot weaken the retained regression baseline.
- Lineage records current, retained, and cumulative masks plus the baseline generation.
- TESTS module declares retained tests as promotion requirements.

## Compatibility and bounds
- Existing projects without TESTBASE begin with their current v15 test plan and establish a baseline on the next successful promotion.
- The baseline is intentionally append-only in v15. Removing an established behavior requires a future explicit test-retirement mechanism; silently dropping a test is not allowed.
- Tests remain bounded VM-level transition tests. They do not inject physical keyboard or mouse events.
- Generated FBC3 remains isolated in Ring-3 ForgeRunner and has no raw PCI, DMA, IRQ, or driver authority.

Physical target validation is still required.
