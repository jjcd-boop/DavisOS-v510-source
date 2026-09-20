#!/usr/bin/env python3
"""Davis OS v360 qualification gate.
Host-side deterministic release gate. It never promotes a candidate and never mutates /Home.
"""
from __future__ import annotations
import argparse, hashlib, json, pathlib, py_compile, subprocess, sys, time, re

ROOT_MARKERS=("Kernel/Main.cpp","Boot/Main.cpp","Apps/Forge/main.cpp","Tools/system-builder/system_builder.py")
REQUIRED_SECURITY=(
 ("Kernel/DxeLoader.cpp","domain=SecurityDomain::Kind::Application","DXE2 defaults to Ring-3 application"),
 ("Kernel/DxeLoader.cpp","SecurityPolicy::GrantFor","kernel mediates executable capabilities"),
 ("Kernel/KernelSecurity.cpp","FatalIntegrityFault","integrity faults fail closed"),
 ("Kernel/DesktopRuntime.cpp","Input::Init(state.input","desktop input baseline present"),
)
FORBIDDEN_DESKTOP=("NetworkService::PumpOnce","AudioService::PumpOnce","StorageService::PumpOnce")

def sha256(p:pathlib.Path):
 h=hashlib.sha256()
 with p.open('rb') as f:
  for b in iter(lambda:f.read(1024*1024),b''):h.update(b)
 return h.hexdigest()

def run(cmd,cwd,timeout=180):
 try:
  p=subprocess.run(cmd,cwd=cwd,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,timeout=timeout)
  return p.returncode,p.stdout[-16000:]
 except subprocess.TimeoutExpired as e:
  return 124,(e.stdout or '')[-16000:]+'\nTIMEOUT'

def add(checks,name,ok,detail=''):
 checks.append({'name':name,'state':'PASS' if ok else 'FAIL','detail':detail})

def main():
 ap=argparse.ArgumentParser();ap.add_argument('--source',type=pathlib.Path,required=True);ap.add_argument('--out',type=pathlib.Path,required=True);ap.add_argument('--skip-build',action='store_true');a=ap.parse_args()
 root=a.source.resolve();a.out.mkdir(parents=True,exist_ok=True);checks=[];logs=[]
 for rel in ROOT_MARKERS:add(checks,'tree:'+rel,(root/rel).is_file())
 for rel,needle,label in REQUIRED_SECURITY:
  p=root/rel;add(checks,'security:'+label,p.is_file() and needle in p.read_text(errors='replace'))
 dr=(root/'Kernel/DesktopRuntime.cpp').read_text(errors='replace')
 add(checks,'isolation:desktop-does-not-pump-services',not any(x in dr for x in FORBIDDEN_DESKTOP))
 # Forge tooling must remain syntactically executable.
 pyfiles=sorted((root/'Tools/system-builder').glob('*.py')) + sorted((root/'Tools/qualification').glob('*.py'))
 syntax_ok=True
 for p in pyfiles:
  try: py_compile.compile(str(p),doraise=True)
  except Exception as e: syntax_ok=False;logs.append({'python':str(p),'error':str(e)})
 add(checks,'forge:python-syntax',syntax_ok,f'{len(pyfiles)} modules')
 rc,out=run([sys.executable,'Tools/system-builder/system_builder.py','--stage','/dev/null','--source',str(root),'--out',str(a.out/'negative-stage'),'--no-build'],root)
 # Invalid transaction must fail closed (nonzero), regardless of exact parse failure.
 add(checks,'forge:invalid-stage-fails-closed',rc!=0,f'rc={rc}');logs.append({'negative_stage':out})
 rc,out=run([sys.executable,'Tools/system-builder/system_dependency_planner.py','--goal','rewrite the entire kernel without constraints','--out',str(a.out/'negative-goal')],root)
 add(checks,'forge:unbounded-goal-fails-closed',rc!=0,f'rc={rc}');logs.append({'negative_goal':out})
 # v28 Forge must expose bounded scheduler tuning but reject unsafe values.
 sys.path.insert(0,str(root/'Tools/system-builder'))
 try:
  import system_builder as sb
  try:
   sb.synthesize_patch({'component':1,'request':'set smp handoff magic to 0'},root); ap_abi_unsafe=False
  except sb.Reject: ap_abi_unsafe=True
  add(checks,'forge:ap-handoff-abi-protected',ap_abi_unsafe)
  try:
   sb.synthesize_patch({'component':1,'request':'change ap cr3 limit to 64 bit'},root); ap_cr3_unsafe=False
  except sb.Reject: ap_cr3_unsafe=True
  add(checks,'forge:ap-prepaging-cr3-contract-protected',ap_cr3_unsafe)
  fake={'component':1,'request':'set scheduler quantum ticks to 8'}
  patch=sb.synthesize_patch(fake,root)
  add(checks,'forge:scheduler-recipe', 'Kernel/Scheduler.hpp' in patch and 'cfg={8}' in patch)
  try:
   sb.synthesize_patch({'component':1,'request':'set scheduler quantum ticks to 1000'},root); unsafe_rejected=False
  except sb.Reject: unsafe_rejected=True
  add(checks,'forge:scheduler-bound-reject',unsafe_rejected)
  patch=sb.synthesize_patch({'component':1,'request':'set tcp maximum sockets to 16'},root)
  add(checks,'forge:tcp-socket-recipe','Kernel/Tcp.hpp' in patch and 'MaxSockets=16' in patch)
  try:
   sb.synthesize_patch({'component':1,'request':'set tcp maximum sockets to 1000'},root); tcp_unsafe=False
  except sb.Reject: tcp_unsafe=True
  add(checks,'forge:tcp-socket-bound-reject',tcp_unsafe)
  patch=sb.synthesize_patch({'component':1,'request':'set maximum file size to 32768'},root)
  add(checks,'forge:file-size-recipe','Kernel/DxeServices.hpp' in patch and 'MaxFileBytes=32768' in patch)
  try:
   sb.synthesize_patch({'component':1,'request':'set maximum file size to 9999999'},root); file_unsafe=False
  except sb.Reject: file_unsafe=True
  add(checks,'forge:file-size-bound-reject',file_unsafe)
  patch=sb.synthesize_patch({'component':1,'request':'set audio maximum streams to 12'},root)
  add(checks,'forge:audio-stream-recipe','Kernel/AudioCore.hpp' in patch and 'MaxStreams=12' in patch)
  try:
   sb.synthesize_patch({'component':1,'request':'set audio maximum streams to 1000'},root); audio_unsafe=False
  except sb.Reject: audio_unsafe=True
  add(checks,'forge:audio-stream-bound-reject',audio_unsafe)
  patch=sb.synthesize_patch({'component':2,'request':'set usb poll timeout spins to 1000000'},root)
  add(checks,'forge:usb-timeout-recipe','Apps/Drivers/UsbDriver/main.cpp' in patch and 'spins=1000000' in patch)
  try:
   sb.synthesize_patch({'component':2,'request':'set usb poll timeout spins to 99999999'},root); usb_unsafe=False
  except sb.Reject: usb_unsafe=True
  add(checks,'forge:usb-timeout-bound-reject',usb_unsafe)
  patch=sb.synthesize_patch({'component':1,'request':'set storage request depth to 64'},root)
  add(checks,'forge:storage-request-depth-recipe','Kernel/BlockDevice.hpp' in patch and 'MaxRequests=64' in patch)
  try:
   sb.synthesize_patch({'component':1,'request':'set storage request depth to 4096'},root); storage_unsafe=False
  except sb.Reject: storage_unsafe=True
  add(checks,'forge:storage-request-depth-bound-reject',storage_unsafe)
  patch=sb.synthesize_patch({'component':1,'request':'set nvme admin queue depth to 128'},root)
  add(checks,'forge:nvme-queue-recipe','Kernel/Nvme.hpp' in patch and 'MaxAdminQueueDepth=128' in patch)
  try:
   sb.synthesize_patch({'component':1,'request':'set nvme admin queue depth to 4096'},root); nvme_unsafe=False
  except sb.Reject: nvme_unsafe=True
  add(checks,'forge:nvme-queue-bound-reject',nvme_unsafe)
  patch=sb.synthesize_patch({'component':1,'request':'set smp ap startup attempts to 4'},root)
  add(checks,'forge:smp-startup-recipe','Kernel/SmpTopology.hpp' in patch and 'MaxStartupAttempts=4' in patch)
  try:
   sb.synthesize_patch({'component':1,'request':'set smp ap startup attempts to 99'},root); smp_unsafe=False
  except sb.Reject: smp_unsafe=True
  add(checks,'forge:smp-startup-bound-reject',smp_unsafe)
  patch=sb.synthesize_patch({'component':1,'request':'set smp ap stack size to 32768'},root)
  add(checks,'forge:smp-ap-stack-recipe','Kernel/ApBootstrap.hpp' in patch and 'DefaultStackBytes=32768' in patch)
  try:
   sb.synthesize_patch({'component':1,'request':'set smp ap stack size to 12345'},root); smp_stack_unsafe=False
  except sb.Reject: smp_stack_unsafe=True
  add(checks,'forge:smp-ap-stack-bound-reject',smp_stack_unsafe)
  patch=sb.synthesize_patch({'component':1,'request':'set smp ap rendezvous poll limit to 250000'},root)
  add(checks,'forge:smp-rendezvous-poll-recipe','Kernel/SmpBringup.hpp' in patch and 'DefaultRendezvousPollLimit=250000' in patch)
  try:
   sb.synthesize_patch({'component':1,'request':'set smp ap rendezvous poll limit to 999999999'},root); smp_poll_unsafe=False
  except sb.Reject: smp_poll_unsafe=True
  add(checks,'forge:smp-rendezvous-poll-bound-reject',smp_poll_unsafe)
  patch=sb.synthesize_patch({'component':1,'request':'set nvme io queue depth to 192'},root)
  add(checks,'forge:nvme-io-queue-recipe','Kernel/Nvme.hpp' in patch and 'MaxIoQueueDepth=192' in patch)
  try:
   sb.synthesize_patch({'component':1,'request':'set nvme io queue depth to 4096'},root); nvme_io_unsafe=False
  except sb.Reject: nvme_io_unsafe=True
  add(checks,'forge:nvme-io-queue-bound-reject',nvme_io_unsafe)
  patch=sb.synthesize_patch({'component':1,'request':'set nvme admin poll timeout to 2000000'},root)
  add(checks,'forge:nvme-admin-timeout-recipe','Kernel/NvmeAdminRuntime.hpp' in patch and 'DefaultPollLimit=2000000' in patch)
  try:
   sb.synthesize_patch({'component':1,'request':'set nvme admin poll timeout to 999999999'},root); nvme_timeout_unsafe=False
  except sb.Reject: nvme_timeout_unsafe=True
  add(checks,'forge:nvme-admin-timeout-bound-reject',nvme_timeout_unsafe)
  patch=sb.synthesize_patch({'component':1,'request':'set nvme controller ready poll limit to 24000000'},root)
  add(checks,'forge:nvme-controller-ready-poll-recipe','Kernel/NvmeControllerTransport.hpp' in patch and 'DefaultReadyPollLimit=24000000' in patch)
  try:
   sb.synthesize_patch({'component':1,'request':'set nvme controller ready poll limit to 999999999'},root); nvme_controller_poll_unsafe=False
  except sb.Reject: nvme_controller_poll_unsafe=True
  add(checks,'forge:nvme-controller-ready-poll-bound-reject',nvme_controller_poll_unsafe)
  patch=sb.synthesize_patch({'component':1,'request':'set nvme recovery attempts to 5'},root)
  add(checks,'forge:nvme-recovery-attempts-recipe','Kernel/NvmeRecovery.hpp' in patch and 'DefaultMaxResetAttempts=5' in patch)
  try:
   sb.synthesize_patch({'component':1,'request':'set nvme recovery attempts to 99'},root); nvme_recovery_unsafe=False
  except sb.Reject: nvme_recovery_unsafe=True
  add(checks,'forge:nvme-recovery-attempts-bound-reject',nvme_recovery_unsafe)
 except Exception as e:
  add(checks,'forge:scheduler-recipe',False,str(e));add(checks,'forge:scheduler-bound-reject',False,str(e));add(checks,'forge:tcp-socket-recipe',False,str(e));add(checks,'forge:tcp-socket-bound-reject',False,str(e));add(checks,'forge:file-size-recipe',False,str(e));add(checks,'forge:file-size-bound-reject',False,str(e));add(checks,'forge:audio-stream-recipe',False,str(e));add(checks,'forge:audio-stream-bound-reject',False,str(e));add(checks,'forge:usb-timeout-recipe',False,str(e));add(checks,'forge:usb-timeout-bound-reject',False,str(e));add(checks,'forge:storage-request-depth-recipe',False,str(e));add(checks,'forge:storage-request-depth-bound-reject',False,str(e));add(checks,'forge:nvme-queue-recipe',False,str(e));add(checks,'forge:nvme-queue-bound-reject',False,str(e));add(checks,'forge:nvme-io-queue-recipe',False,str(e));add(checks,'forge:nvme-io-queue-bound-reject',False,str(e))
 # Architecture gate: Ring-3 driver sources may not directly contain common x86 privileged I/O instructions.
 priv=re.compile(r'\b(?:cli|sti|hlt|inb|outb|inw|outw|inl|outl)\s*\(',re.I)
 offenders=[]
 for base in (root/'Drivers',root/'Apps'/'Drivers'):
  if base.exists():
   for p in base.rglob('*'):
    if p.suffix.lower() in ('.cpp','.c','.hpp','.h') and priv.search(p.read_text(errors='replace')): offenders.append(str(p.relative_to(root)))
 add(checks,'isolation:ring3-no-direct-privileged-io',not offenders,', '.join(offenders[:8]))
 if not a.skip_build:
  rc,out=run(['bash','./BUILD_ALL_KERNEL_LINUX.sh'],root,300);logs.append({'canonical_build':out});add(checks,'build:canonical',rc==0,f'rc={rc}')
 else:
  add(checks,'build:canonical-prevalidated',(root/'out_all/DAVISKRN.BIN').is_file(),'--skip-build requires existing canonical output')
 k=root/'out_all/DAVISKRN.BIN'; add(checks,'artifact:kernel-present',k.is_file(),str(k))
 efi=root/'out/BOOTX64.EFI'; add(checks,'artifact:uefi-loader-present',efi.is_file(),str(efi))
 if efi.is_file(): add(checks,'artifact:uefi-pe-magic',efi.read_bytes()[:2]==b'MZ')
 elf=root/'out_all/DAVISKRN.ELF'; add(checks,'artifact:kernel-elf-present',elf.is_file(),str(elf))
 if elf.is_file(): add(checks,'artifact:kernel-elf-magic',elf.read_bytes()[:4]==b'\x7fELF')
 if k.is_file(): add(checks,'artifact:kernel-nontrivial',k.stat().st_size>65536,f'{k.stat().st_size} bytes')
 # Existing native allocator regression test, when present, is part of qualification.
 mt=root/'memory_host_test'
 if mt.is_file():
  mt.chmod(mt.stat().st_mode|0o111);rc,out=run([str(mt)],root,30);logs.append({'memory_host_test':out});add(checks,'test:memory-host',rc==0,f'rc={rc}')
 else:add(checks,'test:memory-host',False,'missing')
 # Native scheduler regression: exercise the real scheduler implementation, not a model.
 sched_src=root/'Tests/host/scheduler_regression.cpp'; sched_bin=a.out/'scheduler_regression'
 if sched_src.is_file():
  rc,out=run(['clang++','-std=c++17',str(sched_src),str(root/'Kernel/Scheduler.cpp'),'-I',str(root),'-o',str(sched_bin)],root,60);logs.append({'scheduler_compile':out});add(checks,'test:scheduler-host-compile',rc==0,f'rc={rc}')
  if rc==0:
   rc,out=run([str(sched_bin)],root,30);logs.append({'scheduler_regression':out});add(checks,'test:scheduler-fairness-block-wake-yield',rc==0,f'rc={rc} '+out.strip()[-300:])
  else:add(checks,'test:scheduler-fairness-block-wake-yield',False,'compile failed')
 else:
  add(checks,'test:scheduler-host-compile',False,'missing source');add(checks,'test:scheduler-fairness-block-wake-yield',False,'missing source')
 # Native IPC regression: ownership isolation, bounded queues, revocation and sustained FIFO integrity.
 ipc_src=root/'Tests/host/ipc_regression.cpp'; ipc_bin=a.out/'ipc_regression'
 if ipc_src.is_file():
  rc,out=run(['clang++','-std=c++17',str(ipc_src),str(root/'Kernel/Ipc.cpp'),'-I',str(root),'-o',str(ipc_bin)],root,60);logs.append({'ipc_compile':out});add(checks,'test:ipc-host-compile',rc==0,f'rc={rc}')
  if rc==0:
   rc,out=run([str(ipc_bin)],root,30);logs.append({'ipc_regression':out});add(checks,'test:ipc-isolation-capacity-revoke-fifo',rc==0,f'rc={rc} '+out.strip()[-300:])
  else:add(checks,'test:ipc-isolation-capacity-revoke-fifo',False,'compile failed')
 else:
  add(checks,'test:ipc-host-compile',False,'missing source');add(checks,'test:ipc-isolation-capacity-revoke-fifo',False,'missing source')
 # Native process lifecycle regression: validates malformed-image rejection, bounded live capacity, and safe terminated-slot reuse.
 proc_src=root/'Tests/host/process_lifecycle_regression.cpp'; proc_bin=a.out/'process_lifecycle_regression'
 if proc_src.is_file():
  rc,out=run(['clang++','-std=c++17',str(proc_src),str(root/'Kernel/Process.cpp'),str(root/'Kernel/Dxe.cpp'),'-I',str(root),'-o',str(proc_bin)],root,60);logs.append({'process_lifecycle_compile':out});add(checks,'test:process-lifecycle-host-compile',rc==0,f'rc={rc}')
  if rc==0:
   rc,out=run([str(proc_bin)],root,30);logs.append({'process_lifecycle_regression':out});add(checks,'test:process-exit-relaunch-capacity',rc==0,f'rc={rc} '+out.strip()[-300:])
  else:add(checks,'test:process-exit-relaunch-capacity',False,'compile failed')
 else:
  add(checks,'test:process-lifecycle-host-compile',False,'missing source');add(checks,'test:process-exit-relaunch-capacity',False,'missing source')
 # v40 major milestone: networking, service recovery, and storage-object integrity.
 for testname,sources,label in [
  ('network_regression',['Tests/host/network_regression.cpp','Kernel/NetworkCore.cpp'],'test:network-udp-checksum-fragment-stress'),
  ('service_supervisor_regression',['Tests/host/service_supervisor_regression.cpp','Kernel/ServiceSupervisor.cpp'],'test:service-timeout-restart-quarantine'),
  ('storage_object_regression',['Tests/host/storage_object_regression.cpp','Kernel/DxeServices.cpp','Kernel/ObjectHandles.cpp','Kernel/GraphicsObjectManager.cpp','Kernel/PhysicalMemory.cpp'],'test:storage-object-integrity-revoke')]:
  srcs=[str(root/x) for x in sources]; binp=a.out/testname
  rc,out=run(['clang++','-std=c++17',*srcs,'-I',str(root),'-o',str(binp)],root,60);logs.append({testname+'_compile':out});add(checks,label+'-compile',rc==0,f'rc={rc}')
  if rc==0:
   rc,out=run([str(binp)],root,30);logs.append({testname:out});add(checks,label,rc==0,f'rc={rc} '+out.strip()[-300:])
  else:add(checks,label,False,'compile failed')
 # v50 major milestone: repeated cross-subsystem resource teardown with survivor isolation.
 rt_src=root/'Tests/host/resource_teardown_regression.cpp'; rt_bin=a.out/'resource_teardown_regression'
 rt_sources=[str(rt_src),str(root/'Kernel/Ipc.cpp'),str(root/'Kernel/ObjectHandles.cpp'),str(root/'Kernel/SystemServices.cpp'),str(root/'Kernel/DxeServices.cpp'),str(root/'Kernel/GraphicsObjectManager.cpp'),str(root/'Kernel/PhysicalMemory.cpp')]
 rc,out=run(['clang++','-std=c++17',*rt_sources,'-I',str(root),'-o',str(rt_bin)],root,60);logs.append({'resource_teardown_compile':out});add(checks,'test:resource-teardown-host-compile',rc==0,f'rc={rc}')
 if rc==0:
  rc,out=run([str(rt_bin)],root,30);logs.append({'resource_teardown':out});add(checks,'test:resource-teardown-survivor-isolation',rc==0,f'rc={rc} '+out.strip()[-300:])
 else:add(checks,'test:resource-teardown-survivor-isolation',False,'compile failed')
 # v70 major milestone: deterministic IPv4 route selection and fail-closed gateway behavior.
 route_src=root/'Tests/host/network_route_regression.cpp';route_bin=a.out/'network_route_regression'
 rc,out=run(['clang++','-std=c++17',str(route_src),str(root/'Kernel/NetworkCore.cpp'),'-I',str(root),'-o',str(route_bin)],root,60);logs.append({'network_route_compile':out});add(checks,'test:network-route-host-compile',rc==0,f'rc={rc}')
 if rc==0:
  rc,out=run([str(route_bin)],root,30);logs.append({'network_route':out});add(checks,'test:network-route-subnet-gateway-failclosed',rc==0,f'rc={rc} '+out.strip()[-300:])
 else:add(checks,'test:network-route-subnet-gateway-failclosed',False,'compile failed')
 # v80 major milestone: block-device durability semantics and sustained read/write/flush integrity.
 bd_src=root/'Tests/host/block_device_regression.cpp';bd_bin=a.out/'block_device_regression'
 rc,out=run(['clang++','-std=c++17',str(bd_src),str(root/'Kernel/BlockDevice.cpp'),'-I',str(root),'-o',str(bd_bin)],root,60);logs.append({'block_device_compile':out});add(checks,'test:block-device-host-compile',rc==0,f'rc={rc}')
 if rc==0:
  rc,out=run([str(bd_bin)],root,30);logs.append({'block_device':out});add(checks,'test:block-device-integrity-flush-stress',rc==0,f'rc={rc} '+out.strip()[-300:])
 else:add(checks,'test:block-device-integrity-flush-stress',False,'compile failed')
 # v90 major milestone: filesystem namespace safety and lifecycle durability semantics.
 fs_src=root/'Tests/host/filesystem_lifecycle_regression.cpp';fs_bin=a.out/'filesystem_lifecycle_regression'
 fs_sources=[str(fs_src),str(root/'Kernel/DxeServices.cpp'),str(root/'Kernel/ObjectHandles.cpp'),str(root/'Kernel/GraphicsObjectManager.cpp'),str(root/'Kernel/PhysicalMemory.cpp')]
 rc,out=run(['clang++','-std=c++17',*fs_sources,'-I',str(root),'-o',str(fs_bin)],root,60);logs.append({'filesystem_lifecycle_compile':out});add(checks,'test:filesystem-lifecycle-host-compile',rc==0,f'rc={rc}')
 if rc==0:
  rc,out=run([str(fs_bin)],root,30);logs.append({'filesystem_lifecycle':out});add(checks,'test:filesystem-path-seek-rename-delete-stress',rc==0,f'rc={rc} '+out.strip()[-300:])
 else:add(checks,'test:filesystem-path-seek-rename-delete-stress',False,'compile failed')
 # v100 major milestone: ACPI trust-boundary validation and corrupt-table rejection.
 acpi_src=root/'Tests/host/acpi_tables_regression.cpp';acpi_bin=a.out/'acpi_tables_regression'
 rc,out=run(['clang++','-std=c++17',str(acpi_src),str(root/'Kernel/AcpiTables.cpp'),'-I',str(root),'-o',str(acpi_bin)],root,60);logs.append({'acpi_tables_compile':out});add(checks,'test:acpi-tables-host-compile',rc==0,f'rc={rc}')
 if rc==0:
  rc,out=run([str(acpi_bin)],root,30);logs.append({'acpi_tables':out});add(checks,'test:acpi-checksum-xsdt-fadt-corrupt-reject',rc==0,f'rc={rc} '+out.strip()[-300:])
 else:add(checks,'test:acpi-checksum-xsdt-fadt-corrupt-reject',False,'compile failed')
 # v110 major milestone: ACPI MADT topology parsing for multicore/IOAPIC groundwork.
 madt_src=root/'Tests/host/acpi_madt_regression.cpp';madt_bin=a.out/'acpi_madt_regression'
 rc,out=run(['clang++','-std=c++17',str(madt_src),str(root/'Kernel/AcpiTables.cpp'),str(root/'Kernel/AcpiMadt.cpp'),'-I',str(root),'-o',str(madt_bin)],root,60);logs.append({'acpi_madt_compile':out});add(checks,'test:acpi-madt-host-compile',rc==0,f'rc={rc}')
 if rc==0:
  rc,out=run([str(madt_bin)],root,30);logs.append({'acpi_madt':out});add(checks,'test:acpi-madt-topology-ioapic-override',rc==0,f'rc={rc} '+out.strip()[-300:])
 else:add(checks,'test:acpi-madt-topology-ioapic-override',False,'compile failed')
 # v120 major milestone: bounded SMP CPU lifecycle/state-machine qualification.
 smp_src=root/'Tests/host/smp_topology_regression.cpp';smp_bin=a.out/'smp_topology_regression'
 rc,out=run(['clang++','-std=c++17',str(smp_src),str(root/'Kernel/SmpTopology.cpp'),'-I',str(root),'-o',str(smp_bin)],root,60);logs.append({'smp_topology_compile':out});add(checks,'test:smp-topology-host-compile',rc==0,f'rc={rc}')
 if rc==0:
  rc,out=run([str(smp_bin)],root,30);logs.append({'smp_topology':out});add(checks,'test:smp-topology-startup-retry-stress',rc==0,f'rc={rc} '+out.strip()[-300:])
 else:add(checks,'test:smp-topology-startup-retry-stress',False,'compile failed')
 # v130 major milestone: per-CPU runtime accounting and online/offline lifecycle invariants.
 smpr_src=root/'Tests/host/smp_runtime_regression.cpp';smpr_bin=a.out/'smp_runtime_regression'
 rc,out=run(['clang++','-std=c++17',str(smpr_src),str(root/'Kernel/SmpTopology.cpp'),str(root/'Kernel/SmpRuntime.cpp'),'-I',str(root),'-o',str(smpr_bin)],root,60);logs.append({'smp_runtime_compile':out});add(checks,'test:smp-runtime-host-compile',rc==0,f'rc={rc}')
 if rc==0:
  rc,out=run([str(smpr_bin)],root,30);logs.append({'smp_runtime':out});add(checks,'test:smp-runtime-percpu-accounting-stress',rc==0,f'rc={rc} '+out.strip()[-300:])
 else:add(checks,'test:smp-runtime-percpu-accounting-stress',False,'compile failed')
 # v140 major milestone: SMP-safe spinlock primitives under real host thread contention.
 sync_src=root/'Tests/host/smp_sync_regression.cpp';sync_bin=a.out/'smp_sync_regression'
 rc,out=run(['clang++','-std=c++17','-pthread',str(sync_src),str(root/'Kernel/SmpSync.cpp'),'-I',str(root),'-o',str(sync_bin)],root,60);logs.append({'smp_sync_compile':out});add(checks,'test:smp-sync-host-compile',rc==0,f'rc={rc}')
 if rc==0:
  rc,out=run([str(sync_bin)],root,30);logs.append({'smp_sync':out});add(checks,'test:smp-sync-thread-contention-stress',rc==0,f'rc={rc} '+out.strip()[-300:])
 else:add(checks,'test:smp-sync-thread-contention-stress',False,'compile failed')
 # v150 major milestone: concurrent SMP scheduler gate and NVMe namespace I/O command validation.
 smps_src=root/'Tests/host/smp_scheduler_regression.cpp';smps_bin=a.out/'smp_scheduler_regression'
 rc,out=run(['clang++','-std=c++17','-pthread',str(smps_src),str(root/'Kernel/SmpScheduler.cpp'),str(root/'Kernel/SmpSync.cpp'),str(root/'Kernel/SmpRuntime.cpp'),str(root/'Kernel/SmpTopology.cpp'),str(root/'Kernel/Scheduler.cpp'),'-I',str(root),'-o',str(smps_bin)],root,60);logs.append({'smp_scheduler_compile':out});add(checks,'test:smp-scheduler-host-compile',rc==0,f'rc={rc}')
 if rc==0:
  rc,out=run([str(smps_bin)],root,30);logs.append({'smp_scheduler':out});add(checks,'test:smp-scheduler-concurrent-dispatch',rc==0,f'rc={rc} '+out.strip()[-300:])
 else:add(checks,'test:smp-scheduler-concurrent-dispatch',False,'compile failed')
 nv_src=root/'Tests/host/nvme_io_regression.cpp';nv_bin=a.out/'nvme_io_regression'
 rc,out=run(['clang++','-std=c++17',str(nv_src),str(root/'Kernel/Nvme.cpp'),str(root/'Kernel/Dma.cpp'),str(root/'Kernel/PhysicalMemory.cpp'),str(root/'Kernel/Pci.cpp'),'-I',str(root),'-o',str(nv_bin)],root,60);logs.append({'nvme_io_compile':out});add(checks,'test:nvme-io-host-compile',rc==0,f'rc={rc}')
 if rc==0:
  rc,out=run([str(nv_bin)],root,30);logs.append({'nvme_io':out});add(checks,'test:nvme-io-command-bounds-stress',rc==0,f'rc={rc} '+out.strip()[-300:])
 else:add(checks,'test:nvme-io-command-bounds-stress',False,'compile failed')
 # v160 major milestone: bounded NVMe I/O queue lifecycle, wrap/reuse, saturation, completion validation.
 nvq_src=root/'Tests/host/nvme_queue_lifecycle_regression.cpp';nvq_bin=a.out/'nvme_queue_lifecycle_regression'
 rc,out=run(['clang++','-std=c++17',str(nvq_src),str(root/'Kernel/Nvme.cpp'),str(root/'Kernel/Dma.cpp'),str(root/'Kernel/PhysicalMemory.cpp'),str(root/'Kernel/Pci.cpp'),'-I',str(root),'-o',str(nvq_bin)],root,60);logs.append({'nvme_queue_compile':out});add(checks,'test:nvme-queue-host-compile',rc==0,f'rc={rc}')
 if rc==0:
  rc,out=run([str(nvq_bin)],root,30);logs.append({'nvme_queue':out});add(checks,'test:nvme-queue-lifecycle-stress',rc==0,f'rc={rc} '+out.strip()[-300:])
 else:add(checks,'test:nvme-queue-lifecycle-stress',False,'compile failed')
 # v170 major milestone: concrete NVMe SQ/CQ ring publication, phase handling and fail-closed completion consumption.
 nvr_src=root/'Tests/host/nvme_ring_regression.cpp';nvr_bin=a.out/'nvme_ring_regression'
 rc,out=run(['clang++','-std=c++17',str(nvr_src),str(root/'Kernel/Nvme.cpp'),str(root/'Kernel/Dma.cpp'),str(root/'Kernel/PhysicalMemory.cpp'),str(root/'Kernel/Pci.cpp'),'-I',str(root),'-o',str(nvr_bin)],root,60);logs.append({'nvme_ring_compile':out});add(checks,'test:nvme-ring-host-compile',rc==0,f'rc={rc}')
 if rc==0:
  rc,out=run([str(nvr_bin)],root,30);logs.append({'nvme_ring':out});add(checks,'test:nvme-ring-phase-doorbell-stress',rc==0,f'rc={rc} '+out.strip()[-300:])
 else:add(checks,'test:nvme-ring-phase-doorbell-stress',False,'compile failed')
 # v180 major milestone: multi-page NVMe PRP-list planning and controller doorbell offset validation.
 nvp_src=root/'Tests/host/nvme_prp_regression.cpp';nvp_bin=a.out/'nvme_prp_regression'
 rc,out=run(['clang++','-std=c++17',str(nvp_src),str(root/'Kernel/Nvme.cpp'),str(root/'Kernel/Dma.cpp'),str(root/'Kernel/PhysicalMemory.cpp'),str(root/'Kernel/Pci.cpp'),'-I',str(root),'-o',str(nvp_bin)],root,60);logs.append({'nvme_prp_compile':out});add(checks,'test:nvme-prp-host-compile',rc==0,f'rc={rc}')
 if rc==0:
  rc,out=run([str(nvp_bin)],root,30);logs.append({'nvme_prp':out});add(checks,'test:nvme-prp-multipage-doorbell-stress',rc==0,f'rc={rc} '+out.strip()[-300:])
 else:add(checks,'test:nvme-prp-multipage-doorbell-stress',False,'compile failed')
 # v190 major milestone: pluggable asynchronous block backend used to bridge NVMe into the storage service.
 bbe_src=root/'Tests/host/block_backend_regression.cpp';bbe_bin=a.out/'block_backend_regression'
 rc,out=run(['clang++','-std=c++17',str(bbe_src),str(root/'Kernel/BlockDevice.cpp'),'-I',str(root),'-o',str(bbe_bin)],root,60);logs.append({'block_backend_compile':out});add(checks,'test:block-backend-host-compile',rc==0,f'rc={rc}')
 if rc==0:
  rc,out=run([str(bbe_bin)],root,30);logs.append({'block_backend':out});add(checks,'test:block-backend-async-integrity-stress',rc==0,f'rc={rc} '+out.strip()[-300:])
 else:add(checks,'test:block-backend-async-integrity-stress',False,'compile failed')

 nbb_src=root/'Tests/host/nvme_block_backend_regression.cpp';nbb_bin=a.out/'nvme_block_backend_regression'
 rc,out=run(['clang++','-std=c++17',str(nbb_src),str(root/'Kernel/NvmeBlockBackend.cpp'),str(root/'Kernel/NvmeMmioTransport.cpp'),str(root/'Kernel/NvmeControllerTransport.cpp'),str(root/'Kernel/Nvme.cpp'),str(root/'Kernel/BlockDevice.cpp'),str(root/'Kernel/Dma.cpp'),str(root/'Kernel/PhysicalMemory.cpp'),str(root/'Kernel/Pci.cpp'),'-I',str(root),'-o',str(nbb_bin)],root,60);logs.append({'nvme_block_backend_compile':out});add(checks,'test:nvme-block-backend-host-compile',rc==0,f'rc={rc}')
 if rc==0:
  rc,out=run([str(nbb_bin)],root,30);logs.append({'nvme_block_backend':out});add(checks,'test:nvme-block-backend-integration-stress',rc==0,f'rc={rc} '+out.strip()[-300:])
 else:add(checks,'test:nvme-block-backend-integration-stress',False,'compile failed')
 # v210 major milestone: x86 AP INIT/SIPI planning plus NVMe controller I/O queue creation contracts.
 aps_src=root/'Tests/host/ap_startup_regression.cpp';aps_bin=a.out/'ap_startup_regression'
 rc,out=run(['clang++','-std=c++17',str(aps_src),str(root/'Kernel/ApStartup.cpp'),'-I',str(root),'-o',str(aps_bin)],root,60);logs.append({'ap_startup_compile':out});add(checks,'test:ap-startup-host-compile',rc==0,f'rc={rc}')
 if rc==0:
  rc,out=run([str(aps_bin)],root,30);logs.append({'ap_startup':out});add(checks,'test:ap-startup-init-sipi-plan-stress',rc==0,f'rc={rc} '+out.strip()[-300:])
 else:add(checks,'test:ap-startup-init-sipi-plan-stress',False,'compile failed')
 nvc_src=root/'Tests/host/nvme_queue_create_regression.cpp';nvc_bin=a.out/'nvme_queue_create_regression'
 rc,out=run(['clang++','-std=c++17',str(nvc_src),str(root/'Kernel/Nvme.cpp'),str(root/'Kernel/Dma.cpp'),str(root/'Kernel/PhysicalMemory.cpp'),str(root/'Kernel/Pci.cpp'),'-I',str(root),'-o',str(nvc_bin)],root,60);logs.append({'nvme_queue_create_compile':out});add(checks,'test:nvme-queue-create-host-compile',rc==0,f'rc={rc}')
 if rc==0:
  rc,out=run([str(nvc_bin)],root,30);logs.append({'nvme_queue_create':out});add(checks,'test:nvme-queue-create-contract-stress',rc==0,f'rc={rc} '+out.strip()[-300:])
 else:add(checks,'test:nvme-queue-create-contract-stress',False,'compile failed')
 # v220 major milestone: AP bootstrap stacks, authenticated rendezvous, topology/runtime online cutover.
 apb_src=root/'Tests/host/ap_bootstrap_regression.cpp';apb_bin=a.out/'ap_bootstrap_regression'
 rc,out=run(['clang++','-std=c++17',str(apb_src),str(root/'Kernel/ApBootstrap.cpp'),str(root/'Kernel/SmpTopology.cpp'),str(root/'Kernel/SmpRuntime.cpp'),'-I',str(root),'-o',str(apb_bin)],root,60);logs.append({'ap_bootstrap_compile':out});add(checks,'test:ap-bootstrap-host-compile',rc==0,f'rc={rc}')
 if rc==0:
  rc,out=run([str(apb_bin)],root,30);logs.append({'ap_bootstrap':out});add(checks,'test:ap-bootstrap-rendezvous-stack-stress',rc==0,f'rc={rc} '+out.strip()[-300:])
 else:add(checks,'test:ap-bootstrap-rendezvous-stack-stress',False,'compile failed')
 # v230 major milestone: authenticated AP trampoline handoff and executable LAPIC INIT/SIPI transport.
 apt_src=root/'Tests/host/ap_trampoline_ipi_regression.cpp';apt_bin=a.out/'ap_trampoline_ipi_regression'
 rc,out=run(['clang++','-std=c++17',str(apt_src),str(root/'Kernel/ApTrampoline.cpp'),str(root/'Kernel/ApStartup.cpp'),str(root/'Kernel/LapicIpi.cpp'),'-I',str(root),'-o',str(apt_bin)],root,60);logs.append({'ap_trampoline_ipi_compile':out});add(checks,'test:ap-trampoline-ipi-host-compile',rc==0,f'rc={rc}')
 if rc==0:
  rc,out=run([str(apt_bin)],root,30);logs.append({'ap_trampoline_ipi':out});add(checks,'test:ap-trampoline-handoff-lapic-delivery-stress',rc==0,f'rc={rc} '+out.strip()[-300:])
 else:add(checks,'test:ap-trampoline-handoff-lapic-delivery-stress',False,'compile failed')
 # v240 major milestone: AP architectural transition contract and NVMe admin queue execution/recovery.
 apa_src=root/'Tests/host/ap_arch_state_regression.cpp';apa_bin=a.out/'ap_arch_state_regression'
 rc,out=run(['clang++','-std=c++17',str(apa_src),str(root/'Kernel/ApArchState.cpp'),'-I',str(root),'-o',str(apa_bin)],root,60);logs.append({'ap_arch_state_compile':out});add(checks,'test:ap-arch-state-host-compile',rc==0,f'rc={rc}')
 if rc==0:
  rc,out=run([str(apa_bin)],root,30);logs.append({'ap_arch_state':out});add(checks,'test:ap-arch-transition-stress',rc==0,f'rc={rc} '+out.strip()[-300:])
 else:add(checks,'test:ap-arch-transition-stress',False,'compile failed')
 nva_src=root/'Tests/host/nvme_admin_runtime_regression.cpp';nva_bin=a.out/'nvme_admin_runtime_regression'
 rc,out=run(['clang++','-std=c++17',str(nva_src),str(root/'Kernel/NvmeAdminRuntime.cpp'),str(root/'Kernel/Nvme.cpp'),str(root/'Kernel/Dma.cpp'),str(root/'Kernel/PhysicalMemory.cpp'),str(root/'Kernel/Pci.cpp'),'-I',str(root),'-o',str(nva_bin)],root,60);logs.append({'nvme_admin_runtime_compile':out});add(checks,'test:nvme-admin-runtime-host-compile',rc==0,f'rc={rc}')
 if rc==0:
  rc,out=run([str(nva_bin)],root,30);logs.append({'nvme_admin_runtime':out});add(checks,'test:nvme-admin-queue-execution-reset-stress',rc==0,f'rc={rc} '+out.strip()[-300:])
 else:add(checks,'test:nvme-admin-queue-execution-reset-stress',False,'compile failed')
 # v290 correctness: admin queue creation must ring ADMIN SQ/CQ doorbells (QID 0), not the queues being created.
 nad_src=root/'Tests/host/nvme_admin_doorbell_regression.cpp';nad_bin=a.out/'nvme_admin_doorbell_regression'
 rc,out=run(['clang++','-std=c++17',str(nad_src),str(root/'Kernel/NvmeAdminRuntime.cpp'),str(root/'Kernel/NvmeMmioTransport.cpp'),str(root/'Kernel/Nvme.cpp'),str(root/'Kernel/Dma.cpp'),str(root/'Kernel/PhysicalMemory.cpp'),str(root/'Kernel/Pci.cpp'),'-I',str(root),'-o',str(nad_bin)],root,60);logs.append({'nvme_admin_doorbell_compile':out});add(checks,'test:nvme-admin-doorbell-host-compile',rc==0,f'rc={rc}')
 if rc==0:
  rc,out=run([str(nad_bin)],root,30);logs.append({'nvme_admin_doorbell':out});add(checks,'test:nvme-admin-doorbell-routing',rc==0,f'rc={rc} '+out.strip()[-300:])
 else:add(checks,'test:nvme-admin-doorbell-routing',False,'compile failed')
 # v300 major milestone: controller CAP-derived queue/doorbell contract integrated into MMIO transport.
 nci_src=root/'Tests/host/nvme_capability_integration_regression.cpp';nci_bin=a.out/'nvme_capability_integration_regression'
 rc,out=run(['clang++','-std=c++17',str(nci_src),str(root/'Kernel/NvmeControllerTransport.cpp'),str(root/'Kernel/NvmeMmioTransport.cpp'),str(root/'Kernel/Nvme.cpp'),str(root/'Kernel/Dma.cpp'),str(root/'Kernel/PhysicalMemory.cpp'),str(root/'Kernel/Pci.cpp'),'-I',str(root),'-o',str(nci_bin)],root,60);logs.append({'nvme_capability_integration_compile':out});add(checks,'test:nvme-capability-integration-host-compile',rc==0,f'rc={rc}')
 if rc==0:
  rc,out=run([str(nci_bin)],root,30);logs.append({'nvme_capability_integration':out});add(checks,'test:nvme-capability-derived-mmio-stress',rc==0,f'rc={rc} '+out.strip()[-300:])
 else:add(checks,'test:nvme-capability-derived-mmio-stress',False,'compile failed')
 # v250 major milestone: concrete NVMe MMIO queue transport and bounded controller recovery.
 nmm_src=root/'Tests/host/nvme_mmio_transport_regression.cpp';nmm_bin=a.out/'nvme_mmio_transport_regression'
 rc,out=run(['clang++','-std=c++17',str(nmm_src),str(root/'Kernel/NvmeMmioTransport.cpp'),str(root/'Kernel/Nvme.cpp'),str(root/'Kernel/Dma.cpp'),str(root/'Kernel/PhysicalMemory.cpp'),str(root/'Kernel/Pci.cpp'),'-I',str(root),'-o',str(nmm_bin)],root,60);logs.append({'nvme_mmio_compile':out});add(checks,'test:nvme-mmio-transport-host-compile',rc==0,f'rc={rc}')
 if rc==0:
  rc,out=run([str(nmm_bin)],root,30);logs.append({'nvme_mmio':out});add(checks,'test:nvme-mmio-doorbell-completion-stress',rc==0,f'rc={rc} '+out.strip()[-300:])
 else:add(checks,'test:nvme-mmio-doorbell-completion-stress',False,'compile failed')
 nrc_src=root/'Tests/host/nvme_recovery_regression.cpp';nrc_bin=a.out/'nvme_recovery_regression'
 rc,out=run(['clang++','-std=c++17',str(nrc_src),str(root/'Kernel/NvmeRecovery.cpp'),'-I',str(root),'-o',str(nrc_bin)],root,60);logs.append({'nvme_recovery_compile':out});add(checks,'test:nvme-recovery-host-compile',rc==0,f'rc={rc}')
 if rc==0:
  rc,out=run([str(nrc_bin)],root,30);logs.append({'nvme_recovery':out});add(checks,'test:nvme-bounded-reset-recovery-stress',rc==0,f'rc={rc} '+out.strip()[-300:])
 else:add(checks,'test:nvme-bounded-reset-recovery-stress',False,'compile failed')
 # v260 major milestone: controller register lifecycle plus low-memory AP trampoline image contract.
 nct_src=root/'Tests/host/nvme_controller_transport_regression.cpp';nct_bin=a.out/'nvme_controller_transport_regression'
 rc,out=run(['clang++','-std=c++17',str(nct_src),str(root/'Kernel/NvmeControllerTransport.cpp'),'-I',str(root),'-o',str(nct_bin)],root,60);logs.append({'nvme_controller_transport_compile':out});add(checks,'test:nvme-controller-register-host-compile',rc==0,f'rc={rc}')
 if rc==0:
  rc,out=run([str(nct_bin)],root,30);logs.append({'nvme_controller_transport':out});add(checks,'test:nvme-controller-disable-admin-enable-stress',rc==0,f'rc={rc} '+out.strip()[-300:])
 else:add(checks,'test:nvme-controller-disable-admin-enable-stress',False,'compile failed')
 alm_src=root/'Tests/host/ap_low_memory_regression.cpp';alm_bin=a.out/'ap_low_memory_regression'
 rc,out=run(['clang++','-std=c++17',str(alm_src),str(root/'Kernel/ApLowMemory.cpp'),str(root/'Kernel/ApTrampoline.cpp'),str(root/'Kernel/ApTrampoline16.S'),'-I',str(root),'-o',str(alm_bin)],root,60);logs.append({'ap_low_memory_compile':out});add(checks,'test:ap-low-memory-host-compile',rc==0,f'rc={rc}')
 if rc==0:
  rc,out=run([str(alm_bin)],root,30);logs.append({'ap_low_memory':out});add(checks,'test:ap-low-memory-image-handoff-stress',rc==0,f'rc={rc} '+out.strip()[-300:])
 else:add(checks,'test:ap-low-memory-image-handoff-stress',False,'compile failed')
 # v270 major milestone: executable real-mode/protected-mode/long-mode AP trampoline blob.
 aet_src=root/'Tests/host/ap_executable_trampoline_regression.cpp';aet_bin=a.out/'ap_executable_trampoline_regression'
 rc,out=run(['clang++','-std=c++17',str(aet_src),str(root/'Kernel/ApLowMemory.cpp'),str(root/'Kernel/ApTrampoline.cpp'),str(root/'Kernel/ApTrampoline16.S'),'-I',str(root),'-o',str(aet_bin)],root,60);logs.append({'ap_executable_trampoline_compile':out});add(checks,'test:ap-executable-trampoline-host-compile',rc==0,f'rc={rc}')
 if rc==0:
  rc,out=run([str(aet_bin)],root,30);logs.append({'ap_executable_trampoline':out});add(checks,'test:ap-executable-trampoline-image-stress',rc==0,f'rc={rc} '+out.strip()[-300:])
 else:add(checks,'test:ap-executable-trampoline-image-stress',False,'compile failed')
 # v280 major milestone: integrated BSP-to-AP startup orchestration with authenticated rendezvous mailbox.
 smb_src=root/'Tests/host/smp_bringup_regression.cpp';smb_bin=a.out/'smp_bringup_regression'
 smb_sources=[str(smb_src),str(root/'Kernel/SmpBringup.cpp'),str(root/'Kernel/ApRendezvous.cpp'),str(root/'Kernel/ApBootstrap.cpp'),str(root/'Kernel/SmpTopology.cpp'),str(root/'Kernel/SmpRuntime.cpp'),str(root/'Kernel/ApLowMemory.cpp'),str(root/'Kernel/ApTrampoline.cpp'),str(root/'Kernel/ApStartup.cpp'),str(root/'Kernel/LapicIpi.cpp'),str(root/'Kernel/ApTrampoline16.S')]
 rc,out=run(['clang++','-std=c++17',*smb_sources,'-I',str(root),'-o',str(smb_bin)],root,60);logs.append({'smp_bringup_compile':out});add(checks,'test:smp-bringup-host-compile',rc==0,f'rc={rc}')
 if rc==0:
  rc,out=run([str(smb_bin)],root,30);logs.append({'smp_bringup':out});add(checks,'test:smp-bringup-integrated-rendezvous-stress',rc==0,f'rc={rc} '+out.strip()[-300:])
 else:add(checks,'test:smp-bringup-integrated-rendezvous-stress',False,'compile failed')
 # v340: physical LAPIC register/MSR transport and low-CR3 AP transition safety.
 lhw_src=root/'Tests/host/lapic_hardware_regression.cpp';lhw_bin=a.out/'lapic_hardware_regression'
 rc,out=run(['clang++','-std=c++17',str(lhw_src),str(root/'Kernel/LapicHardware.cpp'),str(root/'Kernel/LapicIpi.cpp'),str(root/'Kernel/ApStartup.cpp'),'-I',str(root),'-o',str(lhw_bin)],root,60);logs.append({'lapic_hardware_compile':out});add(checks,'test:lapic-hardware-host-compile',rc==0,f'rc={rc}')
 if rc==0:
  rc,out=run([str(lhw_bin)],root,30);logs.append({'lapic_hardware':out});add(checks,'test:lapic-hardware-xapic-x2apic-delivery-stress',rc==0,f'rc={rc} '+out.strip()[-300:])
 else:add(checks,'test:lapic-hardware-xapic-x2apic-delivery-stress',False,'compile failed')
 acr_src=root/'Tests/host/ap_low_cr3_regression.cpp';acr_bin=a.out/'ap_low_cr3_regression'
 rc,out=run(['clang++','-std=c++17',str(acr_src),str(root/'Kernel/ApLowMemory.cpp'),str(root/'Kernel/ApTrampoline.cpp'),str(root/'Kernel/ApTrampoline16.S'),'-I',str(root),'-o',str(acr_bin)],root,60);logs.append({'ap_low_cr3_compile':out});add(checks,'test:ap-low-cr3-host-compile',rc==0,f'rc={rc}')
 if rc==0:
  rc,out=run([str(acr_bin)],root,30);logs.append({'ap_low_cr3':out});add(checks,'test:ap-bootstrap-cr3-32bit-transition-boundary',rc==0,f'rc={rc} '+out.strip()[-300:])
 else:add(checks,'test:ap-bootstrap-cr3-32bit-transition-boundary',False,'compile failed')
 try:
  patch=sb.synthesize_patch({'component':1,'request':'set lapic delivery poll limit to 250000'},root)
  add(checks,'forge:lapic-delivery-poll-recipe','Kernel/LapicHardware.hpp' in patch and 'DefaultDeliveryPollLimit=250000' in patch)
  try:
   sb.synthesize_patch({'component':1,'request':'set lapic delivery poll limit to 999999999'},root); lapic_unsafe=False
  except sb.Reject: lapic_unsafe=True
  add(checks,'forge:lapic-delivery-poll-bound-reject',lapic_unsafe)
 except Exception as e:
  add(checks,'forge:lapic-delivery-poll-recipe',False,str(e));add(checks,'forge:lapic-delivery-poll-bound-reject',False,str(e))
 # v360: pre-paging AP handoff authentication and relocation-free trampoline proof.
 apt_src=root/'Tests/host/ap_trampoline_prepaging_regression.cpp';apt_bin=a.out/'ap_trampoline_prepaging_regression'
 rc,out=run(['clang++','-std=c++17',str(apt_src),str(root/'Kernel/ApTrampoline.cpp'),'-I',str(root),'-o',str(apt_bin)],root,60);logs.append({'ap_trampoline_prepaging_compile':out});add(checks,'test:ap-trampoline-prepaging-host-compile',rc==0,f'rc={rc}')
 if rc==0:
  rc,out=run([str(apt_bin)],root,30);logs.append({'ap_trampoline_prepaging':out});add(checks,'test:ap-trampoline-prepaging-auth-stress',rc==0,f'rc={rc} '+out.strip()[-300:])
 else:add(checks,'test:ap-trampoline-prepaging-auth-stress',False,'compile failed')
 tramp_obj=a.out/'ap_trampoline16.o'
 rc,out=run(['clang','-c',str(root/'Kernel/ApTrampoline16.S'),'-o',str(tramp_obj)],root,60);logs.append({'ap_trampoline_object_compile':out});add(checks,'test:ap-trampoline-object-compile',rc==0,f'rc={rc}')
 if rc==0:
  rc,out=run(['readelf','-r',str(tramp_obj)],root,30);logs.append({'ap_trampoline_relocations':out});add(checks,'test:ap-trampoline-relocation-free',rc==0 and 'no relocations' in out.lower(),out.strip()[-300:])
 else:add(checks,'test:ap-trampoline-relocation-free',False,'compile failed')
 # v370: assembly-verifiable pre-paging integrity seal protects stack/entry/token/generation before use.
 apsl_src=root/'Tests/host/ap_prepaging_seal_regression.cpp';apsl_bin=a.out/'ap_prepaging_seal_regression'
 rc,out=run(['clang++','-std=c++17',str(apsl_src),str(root/'Kernel/ApTrampoline.cpp'),'-I',str(root),'-o',str(apsl_bin)],root,60);logs.append({'ap_prepaging_seal_compile':out});add(checks,'test:ap-prepaging-seal-host-compile',rc==0,f'rc={rc}')
 if rc==0:
  rc,out=run([str(apsl_bin)],root,30);logs.append({'ap_prepaging_seal':out});add(checks,'test:ap-prepaging-seal-critical-field-corruption',rc==0,f'rc={rc} '+out.strip()[-300:])
 else:add(checks,'test:ap-prepaging-seal-critical-field-corruption',False,'compile failed')
 try:
  sb.synthesize_patch({'component':1,'request':'disable ap pre-paging integrity seal'},root); seal_unsafe=False
 except sb.Reject: seal_unsafe=True
 add(checks,'forge:ap-prepaging-seal-protected',seal_unsafe)
 # v350: bind trusted MADT/APIC platform facts to the concrete LAPIC hardware transport.
 slp_src=root/'Tests/host/smp_lapic_platform_regression.cpp';slp_bin=a.out/'smp_lapic_platform_regression'
 rc,out=run(['clang++','-std=c++17',str(slp_src),str(root/'Kernel/SmpLapicPlatform.cpp'),str(root/'Kernel/LapicHardware.cpp'),str(root/'Kernel/LapicIpi.cpp'),str(root/'Kernel/ApStartup.cpp'),'-I',str(root),'-o',str(slp_bin)],root,60);logs.append({'smp_lapic_platform_compile':out});add(checks,'test:smp-lapic-platform-host-compile',rc==0,f'rc={rc}')
 if rc==0:
  rc,out=run([str(slp_bin)],root,30);logs.append({'smp_lapic_platform':out});add(checks,'test:smp-lapic-platform-firmware-hardware-binding',rc==0,f'rc={rc} '+out.strip()[-300:])
 else:add(checks,'test:smp-lapic-platform-firmware-hardware-binding',False,'compile failed')
 # v60 major milestone: protocol hardening and reusable TCP socket lifecycle.
 for testname,sources,label in [
  ('network_protocol_hardening_regression',['Tests/host/network_protocol_hardening_regression.cpp','Kernel/NetworkCore.cpp','Kernel/Tcp.cpp','Kernel/Dns.cpp','Kernel/Dhcp.cpp'],'test:network-protocol-hardening'),
  ('tcp_socket_lifecycle_regression',['Tests/host/tcp_socket_lifecycle_regression.cpp','Kernel/NetworkCore.cpp','Kernel/Tcp.cpp'],'test:tcp-socket-lifecycle')]:
  srcs=[str(root/x) for x in sources];binp=a.out/testname
  rc,out=run(['clang++','-std=c++17',*srcs,'-I',str(root),'-o',str(binp)],root,60);logs.append({testname+'_compile':out});add(checks,label+'-compile',rc==0,f'rc={rc}')
  if rc==0:
   rc,out=run([str(binp)],root,30);logs.append({testname:out});add(checks,label,rc==0,f'rc={rc} '+out.strip()[-300:])
  else:add(checks,label,False,'compile failed')

 # v310 major milestone: StorageService-integrable NVMe I/O queue runtime.
 nsr_src=root/'Tests/host/nvme_storage_runtime_regression.cpp';nsr_bin=a.out/'nvme_storage_runtime_regression'
 rc,out=run(['clang++','-std=c++17','-pthread',str(nsr_src),str(root/'Kernel/NvmeStorageRuntime.cpp'),str(root/'Kernel/NvmeAdminRuntime.cpp'),str(root/'Kernel/NvmeMmioTransport.cpp'),str(root/'Kernel/NvmeControllerTransport.cpp'),str(root/'Kernel/Nvme.cpp'),str(root/'Kernel/Dma.cpp'),str(root/'Kernel/PhysicalMemory.cpp'),str(root/'Kernel/Pci.cpp'),'-I',str(root),'-o',str(nsr_bin)],root,60);logs.append({'nvme_storage_runtime_compile':out});add(checks,'test:nvme-storage-runtime-host-compile',rc==0,f'rc={rc}')
 if rc==0:
  rc,out=run([str(nsr_bin)],root,30);logs.append({'nvme_storage_runtime':out});add(checks,'test:nvme-storage-runtime-integration',rc==0,f'rc={rc} '+out.strip()[-300:])
 else:add(checks,'test:nvme-storage-runtime-integration',False,'compile failed')
 # v320: full BlockDevice -> bounce DMA -> NVMe MMIO SQ/CQ -> completion data path.
 e2e_src=root/'Tests/host/nvme_end_to_end_storage_regression.cpp';e2e_bin=a.out/'nvme_end_to_end_storage_regression'
 e2e_sources=[str(e2e_src),str(root/'Kernel/NvmeStorageRuntime.cpp'),str(root/'Kernel/NvmeBlockBackend.cpp'),str(root/'Kernel/BlockDevice.cpp'),str(root/'Kernel/NvmeAdminRuntime.cpp'),str(root/'Kernel/NvmeMmioTransport.cpp'),str(root/'Kernel/NvmeControllerTransport.cpp'),str(root/'Kernel/Nvme.cpp'),str(root/'Kernel/Dma.cpp'),str(root/'Kernel/PhysicalMemory.cpp'),str(root/'Kernel/Pci.cpp')]
 rc,out=run(['clang++','-std=c++17','-pthread',*e2e_sources,'-I',str(root),'-o',str(e2e_bin)],root,60);logs.append({'nvme_end_to_end_compile':out});add(checks,'test:nvme-end-to-end-storage-host-compile',rc==0,f'rc={rc}')
 if rc==0:
  rc,out=run([str(e2e_bin)],root,30);logs.append({'nvme_end_to_end':out});add(checks,'test:nvme-end-to-end-bounce-dma-data-integrity',rc==0,f'rc={rc} '+out.strip()[-300:])
 else:add(checks,'test:nvme-end-to-end-bounce-dma-data-integrity',False,'compile failed')
 # v330: removable storage lifecycle: cancel in-flight DMA I/O, unregister device/backend, and safely rebind.
 nhr_src=root/'Tests/host/nvme_hot_rebind_regression.cpp';nhr_bin=a.out/'nvme_hot_rebind_regression'
 nhr_sources=[str(nhr_src),str(root/'Kernel/NvmeBlockBackend.cpp'),str(root/'Kernel/BlockDevice.cpp'),str(root/'Kernel/NvmeStorageRuntime.cpp'),str(root/'Kernel/NvmeAdminRuntime.cpp'),str(root/'Kernel/NvmeMmioTransport.cpp'),str(root/'Kernel/NvmeControllerTransport.cpp'),str(root/'Kernel/Nvme.cpp'),str(root/'Kernel/Dma.cpp'),str(root/'Kernel/PhysicalMemory.cpp'),str(root/'Kernel/Pci.cpp')]
 rc,out=run(['clang++','-std=c++17',*nhr_sources,'-I',str(root),'-o',str(nhr_bin)],root,60);logs.append({'nvme_hot_rebind_compile':out});add(checks,'test:nvme-hot-rebind-host-compile',rc==0,f'rc={rc}')
 if rc==0:
  rc,out=run([str(nhr_bin)],root,30);logs.append({'nvme_hot_rebind':out});add(checks,'test:nvme-hot-rebind-inflight-cancel-stress',rc==0,f'rc={rc} '+out.strip()[-300:])
 else:add(checks,'test:nvme-hot-rebind-inflight-cancel-stress',False,'compile failed')
 try:
  patch=sb.synthesize_patch({'component':1,'request':'set nvme maximum transfer bytes to 524288'},root)
  add(checks,'forge:nvme-max-transfer-recipe','Kernel/NvmeBlockBackend.hpp' in patch and 'MaxBounceBytes=512ull*1024ull' in patch)
  try:
   sb.synthesize_patch({'component':1,'request':'set nvme maximum transfer bytes to 12345'},root); transfer_unsafe=False
  except sb.Reject: transfer_unsafe=True
  add(checks,'forge:nvme-max-transfer-bound-reject',transfer_unsafe)
 except Exception as e:
  add(checks,'forge:nvme-max-transfer-recipe',False,str(e));add(checks,'forge:nvme-max-transfer-bound-reject',False,str(e))
 # v380: physical-machine inventory must surface storage remapping and MADT topology before generic PCI output.
 hp=(root/'Kernel/HardwareProbe.cpp').read_text(errors='ignore')
 pci=(root/'Kernel/Pci.cpp').read_text(errors='ignore')
 add(checks,'test:physical-probe-storage-first', 'classCode==0x01' in hp and 'deviceLine(\"STORAGE\"' in hp and 'storageControllerCount' in hp)
 add(checks,'test:physical-probe-madt-topology', 'AcpiMadt::Parse' in hp and 'APIC IDs:' in hp)
 add(checks,'test:pci-intel-storage-remap-classification', 'intelVmdCandidateFound' in pci and 'd.vendor==0x8086' in pci and 'd.subclass==0x04' in pci)

 # v390: physical evidence follow-up: raw PCI config corroboration and duplicate enabled APIC detection.
 hp390=(root/'Kernel/HardwareProbe.cpp').read_text(errors='ignore')
 add(checks,'test:physical-probe-pci-raw-corroboration', 'pciDetail' in hp390 and 'NET VERIFY matches:' in hp390 and 'matchingBdfCount' in hp390)
 add(checks,'test:physical-probe-apic-duplicate-detection', 'duplicateEnabledApic' in hp390 and 'enabled APIC IDs unique' in hp390)
 add(checks,'test:physical-probe-remains-read-only', 'Read-only inventory; storage activation/writes suppressed' in hp390 and 'WriteConfig' not in hp390)
 # v450: physical Ring-3 cutover regression. xAPIC MMIO at 0xFEE00000 is in the lower canonical range,
 # but must remain supervisor-only while an application CR3 is active.
 sm_src=root/'Tests/host/supervisor_mmio_regression.cpp';sm_bin=a.out/'supervisor_mmio_regression'
 rc,out=run(['clang++','-std=c++17',str(sm_src),str(root/'Kernel/PageTables.cpp'),str(root/'Kernel/DeviceMappings.cpp'),str(root/'Kernel/PhysicalMemory.cpp'),'-I',str(root),'-o',str(sm_bin)],root,60);logs.append({'supervisor_mmio_compile':out});add(checks,'test:ring3-supervisor-mmio-host-compile',rc==0,f'rc={rc}')
 if rc==0:
  rc,out=run([str(sm_bin)],root,30);logs.append({'supervisor_mmio':out});add(checks,'test:ring3-xapic-supervisor-mmio-contract',rc==0,f'rc={rc} '+out.strip()[-300:])
 else:add(checks,'test:ring3-xapic-supervisor-mmio-contract',False,'compile failed')
 dm=(root/'Kernel/DeviceMappings.cpp').read_text(errors='ignore');pt=(root/'Kernel/PageTables.cpp').read_text(errors='ignore')
 add(checks,'test:ring3-mmio-never-user-accessible','MapSupervisor4K' in dm and 'u64 pf=P|(f&W)' in pt)
 # v460: forensic repair - boot only prepares apps; explicit desktop launch owns first CPL3 slice.
 main460=(root/'Kernel/Main.cpp').read_text(errors='ignore');desk460=(root/'Kernel/Desktop.cpp').read_text(errors='ignore');mar460=(root/'Kernel/MultiAppRuntime.cpp').read_text(errors='ignore')
 add(checks,'test:ring3-no-eager-boot-cohort','BootAppRunner::RunGeneric(MultiAppRuntime::state' not in main460)
 add(checks,'test:ring3-explicit-launch-slice','BootAppRunner::LaunchKind' in desk460 and 'bool LaunchKind' in (root/'Kernel/BootAppRunner.cpp').read_text(errors='ignore'))
 add(checks,'test:boot-app-manifest-sealed','BootManifestIntact' in mar460 and 'bootManifestSeal' in mar460)
 passed=all(c['state']=='PASS' for c in checks)
 rec={'qualification':'DAVIS_OS_RELEASE_GATE','version':'460.00','state':'QUALIFIED_HOST' if passed else 'REJECTED','physical_hardware':'REQUIRED_BEFORE_RELEASE','vm_boot':'NOT_RUN_NO_QEMU_AVAILABLE','promotion':'NEVER_AUTOMATIC','created_unix':int(time.time()),'checks':checks,'kernel_sha256':sha256(k) if k.is_file() else None,'logs':logs}
 (a.out/'QUALIFICATION.json').write_text(json.dumps(rec,indent=2)+'\n')
 print(json.dumps({'ok':passed,'state':rec['state'],'passed':sum(c['state']=='PASS' for c in checks),'total':len(checks),'report':str(a.out/'QUALIFICATION.json')},indent=2))
 return 0 if passed else 2
if __name__=='__main__':raise SystemExit(main())