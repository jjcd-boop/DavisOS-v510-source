# DavisOS v490.00 — Ring-3 trap CR3 / first-click freeze repair

Physical clicks still halted the machine after v480's handshake. v480 resumed
the prepared slot (good) and pumped the desktop on yield (partial), but two
independent CPL0 faults remained on the first `int $0x80`.

## Root cause

1. **Syscalls, timer IRQs, and exceptions ran on the process CR3.** Page-table
   walks and `UserHeap::Allocate` identity-map physical pages. Those pages are
   present in the UEFI/kernel CR3 and **absent** from the app CR3 (which only
   maps the 64 MiB kernel image, RSP0, IST, and xAPIC). Writer/Calculator's
   first `SYS_ALLOC` therefore #PF'd in kernel mode → double fault → `cli;hlt`.

2. **Timer IRQ between `mov cr3` and `mov rsp` in `DavisResumePreparedUser`.**
   Interrupts were already live from `TimerRuntime::Activate`. One instruction
   of "user CR3 + UEFI RSP" is enough for the LAPIC to push a frame onto an
   unmapped stack → double fault.

3. **Handshake called `DesktopRuntime::PumpOnce`**, which re-enters
   `Desktop::Update` on the same click (nested launch / full redraw) and left
   `TrapExit.requested` set so a timer during the pump abandoned the slice.

## Fixes (drop these files onto the v480 tree)

- `Kernel/TrapExit.S` — `cli` across CR3/RSP switch; optional mapped RSP0 for
  Enter; `DavisSwitchToKernelAddressSpace` / restore helpers.
- `Kernel/UserEntry.S` — switch to kernel CR3 after building the syscall frame
  on TSS.RSP0; restore user CR3 before `iretq`.
- `Kernel/TimerEntry.S` / `Kernel/Exceptions.S` — same CR3 dance; restore only
  when returning to CPL3 *and* still on the kernel CR3 (context switch may
  already have loaded the next process).
- `Kernel/BootAppRunner.cpp` — `DesktopService::Tick()` only; `TrapExit::Clear()`
  immediately after each slice; wait for `OwnerHasPresented`, not merely a
  window object; prefer Resume over Enter.
- `Kernel/DxeServices.{hpp,cpp}` — `OwnerHasPresented`.

v480's yield-frame snapshot and `TimerRuntime::ResetAfterSlice` are kept.

## Apply

Copy `davisos/Kernel/*` over the corresponding files in the v480 source, rebuild
with `BUILD_DAVIS_OS.sh`, and reflash. Ring-0 Diagnostics stays independent of
Ring-3 apps. Physical storage writes remain disabled.
