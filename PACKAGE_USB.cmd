@echo off
setlocal
cd /d "%~dp0"
if not exist USB_ROOT\EFI\BOOT\BOOTX64.EFI (echo ERROR: Run BUILD_STANDALONE.cmd first.& exit /b 1)
powershell -NoProfile -ExecutionPolicy Bypass -Command "Compress-Archive -Path '.\USB_ROOT\*' -DestinationPath '.\DavisOS_USB_v0.43.zip' -Force"
echo Created DavisOS_USB_v0.43.zip
