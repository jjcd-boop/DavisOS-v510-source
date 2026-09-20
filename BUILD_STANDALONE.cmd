@echo off
setlocal EnableExtensions
cd /d "%~dp0"
echo Davis OS v1.01 - Standalone LLVM Build + DXE2 Runtime Capabilities
where clang++ >nul 2>nul || (echo ERROR: clang++ not found.& exit /b 1)
where lld-link >nul 2>nul || (echo ERROR: lld-link not found.& exit /b 1)
where ld.lld >nul 2>nul || (echo ERROR: ld.lld not found.& exit /b 1)
where llvm-objcopy >nul 2>nul || (echo ERROR: llvm-objcopy not found.& exit /b 1)
if exist out rmdir /s /q out
if exist USB_ROOT rmdir /s /q USB_ROOT
mkdir out
mkdir USB_ROOT\EFI\BOOT
mkdir USB_ROOT\DAVIS
mkdir USB_ROOT\DAVIS\APPS
clang++ -std=c++17 -target x86_64-pc-windows-msvc -ffreestanding -fno-exceptions -fno-rtti -fshort-wchar -c Boot\Main.cpp -o out\boot.obj || exit /b 1
lld-link /subsystem:efi_application /entry:efi_main /nodefaultlib /machine:x64 /out:USB_ROOT\EFI\BOOT\BOOTX64.EFI out\boot.obj || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\Main.cpp -o out\kernel.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\Graphics.cpp -o out\graphics.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\Desktop.cpp -o out\desktop.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\Input.cpp -o out\input.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\WindowManager.cpp -o out\wm.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\Pci.cpp -o out\pci.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\Nvme.cpp -o out\nvme.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\Xhci.cpp -o out\xhci.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\PhysicalMemory.cpp -o out\pmem.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\Dma.cpp -o out\dma.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\KernelMemory.cpp -o out\kmem.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\XhciDma.cpp -o out\xhdma.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\XhciController.cpp -o out\xhctl.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\XhciPorts.cpp -o out\xhports.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\XhciCommands.cpp -o out\xhcmd.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\UsbCore.cpp -o out\usbcore.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\UsbHid.cpp -o out\usbhid.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\Dxe.cpp -o out\dxe.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\Dxe2.cpp -o out\dxe2.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\DxeImports.cpp -o out\dxeimports.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\Process.cpp -o out\process.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\UserMode.cpp -o out\usermode.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\UserLaunch.cpp -o out\userlaunch.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\PlatformState.cpp -o out\platformstate.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\VirtualMemory.cpp -o out\vm.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\Syscall.cpp -o out\syscall.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\DxeServices.cpp -o out\dxeservices.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\DxeCompositor.cpp -o out\dxecompositor.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\Ipc.cpp -o out\ipc.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\SystemServices.cpp -o out\systemservices.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\DesktopService.cpp -o out\desktopservice.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\DesktopRuntime.cpp -o out\desktopruntime.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\SystemTasks.cpp -o out\systemtasks.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\StorageService.cpp -o out\storageservice.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\BlockDevice.cpp -o out\blockdevice.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\ObjectHandles.cpp -o out\handles.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\UserMemory.cpp -o out\usermemory.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\PageTables.cpp -o out\paging.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\DxeLoader.cpp -o out\dxeload.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\AddressSpace.cpp -o out\addrspace.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\DeviceMappings.cpp -o out\devmap.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\InterruptAddressSpace.cpp -o out\irqas.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\Faults.cpp -o out\faults.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\SyscallAbi.cpp -o out\sysabi.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\Idt.cpp -o out\idt.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\UserTrap.cpp -o out\utrap.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\Exceptions.cpp -o out\exceptions.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\TrapExit.cpp -o out\trapexit.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\AppRuntime.cpp -o out\appruntime.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\MultiAppRuntime.cpp -o out\multiapp.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\Scheduler.cpp -o out\scheduler.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\KernelClock.cpp -o out\kclock.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\Preemption.cpp -o out\preempt.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\PreemptionProof.cpp -o out\preproof.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\RuntimeCurrent.cpp -o out\runtimecurrent.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\LiveProof.cpp -o out\liveproof.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\LiveTrace.cpp -o out\livetrace.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\LiveActivation.cpp -o out\liveactivation.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\LiveDiagnostics.cpp -o out\livediagnostics.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\LiveRunner.cpp -o out\liverunner.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\AppCohort.cpp -o out\appcohort.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\BootAppRunner.cpp -o out\bootapprunner.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\FaultRecovery.cpp -o out\faultrecovery.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\ExceptionRecovery.cpp -o out\exceptionrecovery.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\ContextSwitch.cpp -o out\ctxswitch.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\CommitGuard.cpp -o out\commitguard.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\EmergencyIst.cpp -o out\emergencyist.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\InterruptController.cpp -o out\intctl.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\StackGuard.cpp -o out\stackguard.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\StackRuntime.cpp -o out\stackruntime.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\LocalApicTimer.cpp -o out\lapictimer.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\TimerCalibration.cpp -o out\timercal.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\ApicPlatform.cpp -o out\apicplatform.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\TimerPlatform.cpp -o out\timerplatform.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\TimeReference.cpp -o out\timeref.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\PreemptionHealth.cpp -o out\prehealth.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\TimerActivation.cpp -o out\timeractivate.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\TimerRuntime.cpp -o out\timerruntime.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\LapicCalibration.cpp -o out\lapiccal.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\KernelPlatform.cpp -o out\kplatform.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\PlatformBootstrap.cpp -o out\pbootstrap.o || exit /b 1
clang++ -std=c++17 -target x86_64-unknown-none-elf -ffreestanding -fno-exceptions -fno-rtti -mno-red-zone -fno-builtin -c Kernel\InterruptAudit.cpp -o out\intaudit.o || exit /b 1
clang -target x86_64-unknown-none-elf -ffreestanding -mno-red-zone -c Kernel\UserEntry.S -o out\uentry.o || exit /b 1
clang -target x86_64-unknown-none-elf -ffreestanding -mno-red-zone -c Kernel\Exceptions.S -o out\exceptions_asm.o || exit /b 1
clang -target x86_64-unknown-none-elf -ffreestanding -mno-red-zone -c Kernel\DoubleFault.S -o out\doublefault_asm.o || exit /b 1
clang -target x86_64-unknown-none-elf -ffreestanding -mno-red-zone -c Kernel\TrapExit.S -o out\trapexit_asm.o || exit /b 1
clang -target x86_64-unknown-none-elf -ffreestanding -mno-red-zone -c Kernel\TimerEntry.S -o out\timerentry.o || exit /b 1
ld.lld -T Kernel\kernel.ld -nostdlib -o out\DAVISKRN.ELF out\kernel.o out\graphics.o out\desktop.o out\input.o out\wm.o out\pci.o out\nvme.o out\xhci.o out\pmem.o out\dma.o out\kmem.o out\xhdma.o out\xhctl.o out\xhports.o out\xhcmd.o out\usbcore.o out\usbhid.o out\dxe.o out\dxe2.o out\dxeimports.o out\process.o out\usermode.o out\userlaunch.o out\platformstate.o out\vm.o out\syscall.o out\dxeservices.o out\dxecompositor.o out\ipc.o out\systemservices.o out\desktopservice.o out\desktopruntime.o out\systemtasks.o out\storageservice.o out\blockdevice.o out\handles.o out\usermemory.o out\paging.o out\dxeload.o out\addrspace.o out\devmap.o out\irqas.o out\faults.o out\sysabi.o out\idt.o out\utrap.o out\exceptions.o out\trapexit.o out\appruntime.o out\multiapp.o out\scheduler.o out\kclock.o out\preempt.o out\preproof.o out\runtimecurrent.o out\liveproof.o out\livetrace.o out\liveactivation.o out\livediagnostics.o out\liverunner.o out\appcohort.o out\bootapprunner.o out\faultrecovery.o out\exceptionrecovery.o out\ctxswitch.o out\commitguard.o out\emergencyist.o out\stackguard.o out\stackruntime.o out\intctl.o out\lapictimer.o out\timercal.o out\apicplatform.o out\timerplatform.o out\timeref.o out\prehealth.o out\timeractivate.o out\timerruntime.o out\lapiccal.o out\kplatform.o out\pbootstrap.o out\intaudit.o out\uentry.o out\exceptions_asm.o out\doublefault_asm.o out\trapexit_asm.o out\timerentry.o || exit /b 1
llvm-objcopy -O binary out\DAVISKRN.ELF USB_ROOT\DAVIS\DAVISKRN.BIN || exit /b 1
copy /y Apps\HelloDavis\HelloDavis.dxe USB_ROOT\DAVIS\APPS\HelloDavis.dxe >nul || exit /b 1
copy /y Apps\PreemptionA\PreemptionA.dxe USB_ROOT\DAVIS\APPS\PreemptionA.dxe >nul || exit /b 1
copy /y Apps\PreemptionB\PreemptionB.dxe USB_ROOT\DAVIS\APPS\PreemptionB.dxe >nul || exit /b 1
if exist Apps\FileDemo\FileDemo.dxe copy /y Apps\FileDemo\FileDemo.dxe USB_ROOT\DAVIS\APPS\FileDemo.dxe >nul
echo SUCCESS: USB tree created.
echo USB_ROOT\EFI\BOOT\BOOTX64.EFI
echo USB_ROOT\DAVIS\DAVISKRN.BIN
