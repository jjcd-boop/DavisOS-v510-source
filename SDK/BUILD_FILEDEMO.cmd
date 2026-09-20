@echo off
setlocal
if not exist out mkdir out
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -Iinclude -c crt0.cpp -o out\crt0.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c runtime.cpp -o out\runtime.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -Iinclude -c examples\FileDemo\main.cpp -o out\filedemo.o || exit /b 1
ld.lld -T app.ld -nostdlib out\crt0.o out\runtime.o out\filedemo.o -o out\FileDemo.elf || exit /b 1
python ..\Tools\elf2dxe\elf2dxe.py out\FileDemo.elf -o out\FileDemo.dxe --app-id 0x1001 --cap file-read --cap file-write || exit /b 1
echo SUCCESS: out\FileDemo.dxe
