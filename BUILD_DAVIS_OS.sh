#!/usr/bin/env bash
# Builds Davis OS. This POSIX-shell script is a host-side tool; the target is Davis OS, not Linux.
set -euo pipefail
# Davis OS input regression gate (v1.54+). A build must preserve the tested transport order.
usb_line=$(grep -n -m1 'UsbRuntime::Init();BootDiagnostics::Pass("USB/xHCI input transport returned")' Kernel/Main.cpp | cut -d: -f1 || true)
i2c_line=$(grep -n -m1 'I2cHid::Init' Kernel/Main.cpp | cut -d: -f1 || true)
ps2_line=$(grep -n -m1 'Input::Init(state.input' Kernel/DesktopRuntime.cpp | cut -d: -f1 || true)
if [[ -z "$usb_line" || -z "$i2c_line" || -z "$ps2_line" ]]; then echo 'INPUT REGRESSION GATE: required input initialization marker missing' >&2; exit 91; fi
# PS/2 controller bring-up is a hardware-tested baseline. Changes require deliberate gate update.
expected_ps2='cb596a0d7eeffc739fd4efea3c7339874014153ef478febbbab4c92059159e3c'
actual_ps2=$(sha256sum Kernel/Ps2Diagnostics.cpp | awk '{print $1}')
if [[ "$actual_ps2" != "$expected_ps2" ]]; then echo 'INPUT REGRESSION GATE: Ps2Diagnostics.cpp changed; hardware regression approval required' >&2; exit 92; fi
expected_input='abe4bcb8ff6dcbd921cf05d0bcc1c43b1a5e02f119f9bf063b23d90f8162f890'
actual_input=$(sha256sum Kernel/Input.cpp | awk '{print $1}')
if [[ "$actual_input" != "$expected_input" ]]; then echo 'INPUT REGRESSION GATE: Input.cpp differs from hardware-working v1.48 baseline' >&2; exit 93; fi
if grep -qE 'NetworkService::PumpOnce|AudioService::PumpOnce|StorageService::PumpOnce' Kernel/DesktopRuntime.cpp; then echo 'ISOLATION GATE: desktop runtime must not pump background services' >&2; exit 94; fi
if ! grep -q 'fstack-protector-strong' "$0"; then echo 'KERNEL HARDENING GATE: stack protector flag missing' >&2; exit 95; fi
if ! grep -q 'FatalIntegrityFault' Kernel/KernelSecurity.cpp; then echo 'KERNEL HARDENING GATE: integrity fail-closed path missing' >&2; exit 96; fi
if ! grep -q 'domain=SecurityDomain::Kind::Application' Kernel/DxeLoader.cpp; then echo 'PRIVILEGE GATE: DXE2 default domain must remain Ring-3 application' >&2; exit 97; fi
if ! grep -q 'SecurityPolicy::GrantFor' Kernel/DxeLoader.cpp; then echo 'PRIVILEGE GATE: executable capabilities must pass kernel policy' >&2; exit 98; fi
rm -rf out_all; mkdir out_all
for f in Kernel/*.cpp; do b=$(basename "$f" .cpp); clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -fstack-protector-strong -fno-delete-null-pointer-checks -c "$f" -o "out_all/$b.o"; done
for f in Kernel/*.S; do b=$(basename "$f" .S); clang -target x86_64-unknown-none-elf -ffreestanding -mno-red-zone -c "$f" -o "out_all/${b}_asm.o"; done
ld.lld -T Kernel/kernel.ld -nostdlib -o out_all/DAVISKRN.ELF out_all/*.o
llvm-objcopy -O binary out_all/DAVISKRN.ELF out_all/DAVISKRN.BIN
cp out_all/DAVISKRN.ELF DAVISKRN.ELF
cp out_all/DAVISKRN.BIN DAVISKRN.BIN
mkdir -p out
clang++ -std=c++17 -target x86_64-pc-windows-msvc -ffreestanding -fno-exceptions -fno-rtti -fshort-wchar -c Boot/Main.cpp -o out/boot.obj
lld-link /subsystem:efi_application /entry:efi_main /nodefaultlib /machine:x64 /out:out/BOOTX64.EFI out/boot.obj
