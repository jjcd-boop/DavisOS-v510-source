# DavisOS v480.00 — Ring-3 First-Entry and Window Handshake Repair

Physical-debugging milestone based on the v460/v470 launch-path forensic review.

## Corrected defects
- First CPL3 entry now resumes the already-prepared `Preemption::Slot` frame instead of switching CR3 and then constructing an iretq frame on the inherited UEFI stack.
- Cooperative `Yield` now snapshots the post-syscall user frame into the process preemption slot before abandoning to the kernel.
- Launch is a bounded 32-slice handshake: cooperative/system yields pump the Ring-0 desktop service and resume the saved Ring-3 frame until a window is present or the bound/fault/exit is reached.
- TimerRuntime has an explicit post-slice reset back to `Validated`, with TimerPlatform restored to `Calibrated`, so later application clicks can arm the LAPIC timer again.
- Ring-0 Diagnostics remains independent of Ring-3 applications.

## Safety
- No relaxation of Ring-0/Ring-3 page permissions.
- xAPIC remains supervisor-only.
- Physical storage writes remain disabled in the USB test package.
- Handshake is bounded to avoid an unbounded kernel launch loop.

## Qualification
162/162 host gates pass (`QUALIFIED_HOST`). Physical CPL3/window creation remains to be proven on target hardware.
