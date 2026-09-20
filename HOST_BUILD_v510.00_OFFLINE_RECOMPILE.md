Davis OS v510.00 — offline recompile
=====================================

This tree is self-contained. It does not download anything.
You need a local LLVM toolchain already installed on the build machine
(clang++, clang, ld.lld, llvm-objcopy). No network is required after that.

Linux / macOS
-------------
  1. Confirm tools:
       clang++ --version
       ld.lld --version
       llvm-objcopy --version
  2. From this directory:
       chmod +x BUILD_DAVIS_OS.sh PACKAGE_USB.sh
       ./BUILD_DAVIS_OS.sh
     That produces DAVISKRN.BIN and BOOTX64.EFI in this folder.
  3. Stage a USB tree (still offline):
       ./PACKAGE_USB.sh
     Then copy USB_ROOT/* onto a FAT32 stick so that:
       EFI/BOOT/BOOTX64.EFI
       DAVIS/DAVISKRN.BIN
       DAVIS/APPS/*.dxe
     sit at the volume root. Disable Secure Boot. Boot from USB.

Windows
-------
  Use BUILD_STANDALONE.cmd with the same LLVM tools on PATH
  (clang-cl / lld-link also work for BOOTX64.EFI).

What this build changes versus v480
-----------------------------------
  * Trap CR3 switch on syscall, timer, and exception entry
    (DavisSwitchToKernelAddressSpace). First-click SYS_ALLOC no longer
    walks unmapped physical pages on the process CR3.
  * cli across mov %cr3 / mov %rsp in DavisResumePreparedUser.
  * First-click handshake uses DesktopService::Tick + OwnerHasPresented,
    not a nested DesktopRuntime::PumpOnce.
  * Desktop compositor draws all 16 applications as icons (two columns).
  * Pebble.dxe in Apps/Pebble is the 10 KiB DXE2 image, not the 256 MiB pad.

What this tree deliberately does NOT contain
--------------------------------------------
  * A flashable DAVISKRN.BIN at the root. The 385,392-byte file shipped in
    earlier USB zips was NOT produced by clang in this environment (no
    x86_64-unknown-none-elf toolchain here). Treat it as unverified.
    reference/ holds the v480 unpatched 414,008-byte kernel and the
    unverified 385,392-byte blob for comparison only.
  * Intel AX200/AX201/AX211 firmware. Wi-Fi association on metal still
    needs firmware you supply on the stick if your laptop requires it.
  * npm, node_modules, or any web-preview sources. Those are not the OS.

Offline operation after boot
----------------------------
  The kernel, compositor, Writer, Math Lab, Files, Terminal, Settings,
  Pebble, Morrow, Forge, games, and Media do not need a network.
  WEB/HTTPS is optional and will simply report no route when offline.
  Physical disk writes stay disabled unless you add
  DAVIS/ALLOW_PHYSICAL_WRITES.TXT yourself.

Apps
----
  Prebuilt .dxe images under Apps/*/ are copied by PACKAGE_USB.sh
  (skips files >= 2 MiB). You can also rebuild individual apps with
  the existing per-app scripts if you have clang.

See DAVISOS_ISSUES_AND_CORRECTIONS.txt for the full forensic log.
