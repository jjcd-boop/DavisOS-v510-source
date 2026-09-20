# Davis OS v1.98 — Morrow

Morrow is a native Ring-3 cold-start continuity instrument. Its question is deliberately harsher than backup status: **if this machine vanished now and you had to resume on a blank replacement, what human work would have to be reconstructed?**

Each checkpoint records the current work state, a survival class (RAM only / this machine / reproducible / external copy), where the useful copy lives, estimated minutes required to reconstruct it from nothing, and the smallest clue the user's future self would need. Morrow accumulates those reconstruction estimates as **Cold-Start Exposure** and keeps an append-only local recovery ledger at `/Home/Documents/MORROW.LOG`.

This is not a backup program, autosave utility, task manager, crash-recovery mechanism, or conventional disaster-recovery dashboard. It measures *human reconstruction burden* rather than bytes protected. A file may be safely stored yet still have a high reconstruction burden if the reasoning/context needed to continue the work exists only in the user's current mental state.

## Prior-art challenge
A September 2026 search deliberately looked for system-wide unsaved-state monitors, crash survivability tools, workspace recoverability applications, volatile-state monitors, counterfactual/time-travel tools, and intent/decision-history software. The search found application-specific dirty-state warnings, autosave/session recovery, backup tools, process monitors, and counterfactual agent-debugging systems. It did not reveal an obvious prior desktop application whose primary metric is a user's cross-application *minutes-to-reconstruct-from-a-blank-machine* with explicit survival classes and a minimal future-self recovery clue.

That search cannot logically prove that no private, abandoned, unpublished, patented, or obscure implementation has ever existed. v1.98 therefore treats Morrow as a deliberately novel design with no discovered direct precedent, not as a legally exhaustive claim of worldwide first invention.

## Integration
- BootInfo kind: `AppMorrow = 47`
- UEFI known-app loader: `Morrow.dxe`
- Davis Applications menu: `MORROW`
- DXE2 capabilities: window, IPC, file-read, file-write only
- Persistent ledger: `/Home/Documents/MORROW.LOG`
- Kernel rebuilt with canonical `BUILD_DAVIS_OS.sh`
- UEFI loader rebuilt separately after adding Morrow to the known-app table
