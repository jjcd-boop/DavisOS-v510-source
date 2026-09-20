# Davis OS v6.50 - Forge Interaction and Lifecycle Hardening

This pass closes two concrete runtime defects in the Forge installed-application path.

## Precise generated-control hit testing
ForgeRunner now builds a bounded hit-region table while rendering FBC3 controls. Pointer presses are dispatched only to the control rectangle actually clicked. Input fields receive focus by click; ADD/RESET/CLEAR/action controls execute their own bounded behavior; checkboxes toggle bounded state. The old behavior where an arbitrary click could mutate the counter/list has been removed.

## Installed runner lifecycle
A ForgeRunner started through the installed-package one-shot handoff now exits when its application window closes. The boot ForgeRunner service remains resident. This prevents repeatedly opened installed Forge applications from needlessly consuming the finite runtime process table.

## Package-launch reservation fix
ForgePackageRegistry now verifies/reserves an available pending handoff slot before spawning a new isolated ForgeRunner. This removes the previous failure mode where a runner could be spawned and then orphaned because the pending handoff queue was full.

## Security
No new generated-program authority was introduced. ForgeRunner remains a Ring-3 SystemService with Window + IPC only. Generated FBC3 remains interpreted and cannot request raw PCI, DMA, IRQ, or driver I/O.

## Validation
ForgeRunner was rebuilt as DXE2 app 49 version 6.5 with caps 0x11. The canonical BUILD_DAVIS_OS.sh completed successfully after the kernel registry change. Physical-hardware behavior remains to be validated on the Davis OS target.
