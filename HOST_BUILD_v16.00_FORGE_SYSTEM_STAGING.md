# Davis OS Forge v16.00 — System Staging Foundation

Forge now distinguishes ordinary application generation from explicit Davis OS modification intents.

System intents are fail-closed. Forge writes a versioned `.SYSPROP` proposal describing target, risk, requested change, staging pipeline, known-good preservation, validation requirements, explicit approval, and rollback. v16 deliberately does not patch the running kernel, bootloader, drivers, or promote a system image.

Recognized system targets include Ring-0 kernel, Ring-3 drivers, bootloader, desktop/compositor/window manager, and general Davis OS requests. Kernel/bootloader requests are CRITICAL, driver requests HIGH, and desktop/system-service requests ELEVATED.

Application Forge v15 cumulative regression/evolution behavior remains intact.

Host build: canonical BUILD_DAVIS_OS.sh completed successfully. Physical target validation remains required.
