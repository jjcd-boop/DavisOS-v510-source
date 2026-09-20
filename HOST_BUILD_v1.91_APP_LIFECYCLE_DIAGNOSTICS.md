# Davis OS v1.91 — Application Lifecycle Diagnostics

## Purpose
This revision hardens the desktop/application boundary so a click on a program no longer fails silently when the Ring-3 process is missing, not yet presenting a window, exited, or faulted.

## Changes
- Added per-process `lastFaultVector` telemetry. User-mode exception dispatch records the actual exception vector on the process that faulted before containment/recovery.
- Added per-process `launchAttempts` / focus-attempt telemetry for desktop launch diagnostics.
- Added `MultiAppRuntime::ProcessForKind()` so the shell can inspect the lifecycle state of a prepared Ring-3 application without bypassing process isolation.
- Replaced the shell's old boolean `FocusApp()` behavior with lifecycle-aware focus handling.
- Desktop program activation now distinguishes:
  - process absent from boot image;
  - process prepared/ready but no window created yet;
  - process running but no window exists;
  - process exited;
  - process faulted;
  - normal existing-window focus.
- The shell now displays a diagnostic notice rather than making a failed program click appear to do nothing.

## Why this matters
Previously the desktop only called `RaiseFirstWindowForOwner(pid)`. If no DXE window existed, the function returned false and the result was discarded. A process loader failure, early user-mode exception, normal exit, or a process that had not yet created its window therefore all looked identical to the user: nothing happened.

v1.91 makes those states observable while preserving Ring-3 isolation. It does not paper over faults by silently reopening or replacing a process. This gives the next hardware boot enough information to identify the actual application-launch failure path.

## Build validation
`BUILD_ALL_KERNEL_LINUX.sh` completed successfully after the changes.

## Recommended physical test
Boot v1.91 and click Writer, Calculator, Files, Browser, Terminal, and Settings. Record the exact notice produced by each program that does not open. A `PROCESS FAULTED` result narrows the next pass to exception/address-space/syscall handling; `PREPARED, WINDOW NOT CREATED YET` points toward scheduling; `RUNNING, BUT NO WINDOW EXISTS` points toward DXE service/window creation; `PROCESS EXITED` points toward the application's startup error path.
