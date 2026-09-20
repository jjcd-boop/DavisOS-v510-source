# Davis OS v24.00 — Forge Candidate Boot Checkpoints

Candidate boot recovery now persists bounded UEFI NVRAM checkpoints while a one-shot Forge candidate boots.

Checkpoint sequence:
1. KERNEL_ENTRY
2. MEMORY_READY
3. RUNTIME_READY
4. HARDWARE_READY
5. INPUT_READY
6. DESKTOP_READY
7. RING3_READY
8. HEALTHY

The bootloader still marks ATTEMPTED before transferring control. Each kernel checkpoint updates the same stage-bound record without making the candidate eligible for a second automatic boot. HEALTHY is written only after the generic Ring-3 cohort returns to the desktop path. If the candidate fails before HEALTHY, the retained checkpoint identifies the last completed boot phase on firmware that preserves the variable.

No automatic promotion is implemented. Known-good remains the default after the one-shot candidate attempt.
