# Davis OS v17.00 — Forge Digested System Staging

Forge System Evolution now emits a binary `.SYSSTAGE` transaction plus a human-readable `.SYSPROP` record for OS-level requests.

## Security boundary
- The transaction is declarative and non-executable.
- Each request is classified as kernel, Ring-3 driver, bootloader, desktop/system service, or review-required general OS work.
- Each class has an explicit source allowlist root (`Kernel/`, `Drivers/`, `Boot/`, or `Kernel/Desktop.cpp`).
- The transaction carries request and policy digests plus a whole-record integrity digest.
- Cross-component writes are forbidden by policy.
- Forge remains Ring 3 and receives no raw PCI/DMA/IRQ authority.
- The known-good system must remain immutable until a candidate has passed build, ABI, capability, regression, boot, and rollback gates.
- Promotion remains human-approval-only.

## Important limitation
The target Davis OS image does not contain the complete host build source tree or compiler toolchain. v17 therefore does not claim to rewrite unavailable kernel source on the running machine. A future privileged/offline System Builder can consume the staged transaction, verify the digest and allowlist, build a separate candidate image, and return validation evidence.

This preserves the design rule that Forge never patches the live Ring-0 kernel in place.
