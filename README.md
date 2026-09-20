# Davis OS v510.00 — full standalone source

This is the complete recompile tree (kernel, boot, apps, tools, build scripts).

**Download ZIP:** use GitHub **Code → Download ZIP**, or
https://github.com/jjcd-boop/DavisOS-v510-source/archive/refs/heads/main.zip

## Build (offline)

Needs local LLVM only (`clang++`, `clang`, `ld.lld`, `llvm-objcopy`). The scripts do not download anything.

```
chmod +x BUILD_DAVIS_OS.sh PACKAGE_USB.sh
./BUILD_DAVIS_OS.sh
./PACKAGE_USB.sh
```

Copy `USB_ROOT/` onto a FAT32 stick so `EFI/BOOT/BOOTX64.EFI` is at the volume root. Secure Boot off.

There is **no** flashable `DAVISKRN.BIN` at the repo root until you run the build. See `RECOMPILE.txt` and `DAVISOS_ISSUES_AND_CORRECTIONS.txt`.

## v510 patches vs v480

- `Kernel/TrapExit.S` `UserEntry.S` `TimerEntry.S` `Exceptions.S` — CR3 switch on syscall/timer/exception
- `Kernel/BootAppRunner.cpp` `DxeServices.cpp` `DxeServices.hpp` — first-click handshake
- `Kernel/Desktop.cpp` — all 16 apps as desktop icons
