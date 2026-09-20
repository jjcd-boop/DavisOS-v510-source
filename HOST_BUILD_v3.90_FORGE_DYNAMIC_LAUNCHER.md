# Davis OS v3.90 — Forge Dynamic Installed-App Launcher

This release connects the persistent Forge package registry to the Davis desktop.

- Applications submenu dynamically enumerates installed Forge packages (up to 12).
- Clicking an installed package creates a fresh isolated ForgeRunner process.
- Kernel records a one-shot package handoff keyed to that new PID.
- ForgeRunner fetches its package through syscall 63 after service registration.
- The package is revalidated by the kernel registry before launch and FBC3 is independently validated again by ForgeRunner.
- Generated apps continue to execute as interpreted FBC3 in Ring 3 with Window + IPC only.
- No raw PCI, DMA, IRQ, driver-I/O, arbitrary native execution, or unrestricted filesystem authority is added.
- Dynamic launcher is responsive to 720p and 1080p shell layouts using compact application rows.

Physical-hardware launch behavior remains to be validated on the target laptop.
