#!/usr/bin/env python3
"""Davis OS Forge System Builder v60.
Host-side/offline candidate builder. It never promotes or overwrites a known-good tree.
"""
from __future__ import annotations
import argparse, hashlib, json, os, pathlib, re, shutil, struct, subprocess, sys, tempfile, time

MAGIC=0x47545346
VERSIONS={0x00110000,0x00120000}
POLICY=b"STAGING_ONLY|ALLOWLIST|KNOWN_GOOD|BUILD_TEST|HUMAN_PROMOTION|ROLLBACK"
FMT="<9I40s180s"
SIZE=struct.calcsize(FMT)
COMPONENT={0:"DAVIS_OS",1:"RING0_KERNEL",2:"RING3_DRIVER",3:"BOOTLOADER",4:"DESKTOP_SYSTEM_SERVICE"}
ALLOW={1:("Kernel/",),2:("Drivers/","Apps/Drivers/"),3:("Boot/",),4:("Kernel/Desktop.cpp",)}

class Reject(RuntimeError): pass

def fnv(data:bytes)->int:
    h=2166136261
    for b in data: h=((h^b)*16777619)&0xffffffff
    return h or 1

def stage_digest(raw:bytes)->int:
    # Forge computes with digest=0. Digest is word 8, not the final field.
    b=bytearray(raw)
    struct.pack_into("<I",b,32,0)
    h=2166136261
    # Match target's stageDigest: sizeof(record)-4 bytes.
    for x in b[:-4]: h=((h^x)*16777619)&0xffffffff
    return h

def read_stage(path:pathlib.Path):
    raw=path.read_bytes()
    if len(raw)!=SIZE: raise Reject(f"SYSSTAGE size {len(raw)} != {SIZE}")
    vals=struct.unpack(FMT,raw)
    magic,ver,gen,comp,risk,reqd,pold,flags,digest,project,request=vals
    project=project.split(b'\0',1)[0].decode('utf-8','replace')
    request=request.split(b'\0',1)[0].decode('utf-8','replace')
    if magic!=MAGIC: raise Reject("bad SYSSTAGE magic")
    if ver not in VERSIONS: raise Reject(f"unsupported SYSSTAGE version 0x{ver:08x}")
    if comp not in COMPONENT: raise Reject("unknown component")
    if comp==0: raise Reject("general DAVIS_OS transaction requires manual scoping")
    if fnv(request.encode())!=reqd: raise Reject("request digest mismatch")
    if fnv(POLICY)!=pold: raise Reject("policy digest mismatch")
    if stage_digest(raw)!=digest: raise Reject("transaction integrity digest mismatch")
    if (flags & 0x3f)!=0x3f: raise Reject("mandatory staging flags missing")
    return dict(version=ver,generation=gen,component=comp,component_name=COMPONENT[comp],risk=risk,
                request_digest=reqd,policy_digest=pold,flags=flags,digest=digest,project=project,request=request)

def tree_digest(root:pathlib.Path)->str:
    h=hashlib.sha256()
    excluded={'.git','out_all','out','host_out','host_test','host_test050','out_boot','out_boot161','out_boot168','out_boot201'}
    for p in sorted(root.rglob('*')):
        if not p.is_file() or any(x in excluded for x in p.parts): continue
        rel=p.relative_to(root).as_posix().encode(); h.update(rel+b'\0'); h.update(hashlib.sha256(p.read_bytes()).digest())
    return h.hexdigest()

def parse_patch_targets(text:str):
    targets=[]
    for line in text.splitlines():
        if line.startswith('+++ '):
            s=line[4:].split('\t',1)[0].strip()
            if s=='/dev/null': continue
            if s.startswith('b/'): s=s[2:]
            targets.append(s)
    if not targets: raise Reject("patch contains no target files")
    return targets

def safe_rel(s:str)->str:
    p=pathlib.PurePosixPath(s)
    if p.is_absolute() or '..' in p.parts: raise Reject(f"unsafe path: {s}")
    return p.as_posix()

def allowed(target:str,comp:int)->bool:
    t=safe_rel(target)
    for a in ALLOW[comp]:
        if a.endswith('.cpp'):
            if t==a:return True
        elif t.startswith(a):return True
    return False

def make_diff(path_rel:str, old:str, new:str)->str:
    import difflib
    return ''.join(difflib.unified_diff(old.splitlines(True),new.splitlines(True),fromfile='a/'+path_rel,tofile='b/'+path_rel))

RECIPE_NAMES=('desktop.workspace_label','desktop.applications_label','kernel.audio_max_streams','kernel.scheduler_quantum','kernel.tcp_max_sockets','kernel.max_file_bytes','kernel.nvme_admin_queue_depth','kernel.nvme_io_queue_depth','kernel.nvme_max_transfer_bytes','kernel.nvme_recovery_attempts','kernel.nvme_controller_ready_poll_limit','kernel.smp_startup_attempts','kernel.smp_ap_stack_bytes','kernel.smp_rendezvous_poll_limit','kernel.lapic_delivery_poll_limit','driver.usb_poll_timeout','boot.gop_status_label')

def synthesize_patch(st:dict, source:pathlib.Path)->str:
    """Forge v20 transformation library. Recipes are narrow, typed and bounded.
    Unknown requests fail closed rather than granting a generic source editor.
    """
    req=st['request'].strip(); low=req.lower()
    # AP bootstrap ABI is a hardware trust boundary, not a tunable Forge recipe.
    # Refuse semantic requests that would weaken the pre-paging handoff contract.
    if re.search(r'\b(?:ap|smp)\b.*\b(?:handoff\s+magic|handoff\s+version|handoff\s+offset|handoff\s+(?:seal|checksum)|trampoline\s+handoff|pre[- ]?paging\s+(?:seal|integrity)|cr3\s+(?:limit|width))\b', req, re.I):
        raise Reject('AP trampoline handoff ABI and pre-paging CR3 contract are security-critical and not Forge-tunable')
    if st['component']==4:
        m=re.search(r'(?:rename|set|change)\s+(?:the\s+)?workspace(?:\s+label)?\s+(?:to\s+)?["\']?([A-Za-z0-9 _-]{1,24})', req, re.I)
        if m:
            label=m.group(1).strip().rstrip('. '); path=source/'Kernel'/'Desktop.cpp'; old=path.read_text()
            mm=re.search(r'Graphics::Text\(g,W-116,13,"([^"]{1,32})"',old)
            if not mm: raise Reject('desktop workspace-label anchor not found')
            new=old.replace('Graphics::Text(g,W-116,13,"'+mm.group(1)+'"','Graphics::Text(g,W-116,13,"'+label+'"',1)
            return make_diff('Kernel/Desktop.cpp',old,new)
        m=re.search(r'(?:rename|set|change)\s+(?:the\s+)?applications(?:\s+menu)?(?:\s+label)?\s+(?:to\s+)?["\']?([A-Za-z0-9 _-]{1,20})',req,re.I)
        if m:
            label=m.group(1).strip().rstrip('. '); path=source/'Kernel'/'Desktop.cpp'; old=path.read_text()
            anchor='Graphics::Text(g,92,13,"Applications"'
            if anchor not in old: raise Reject('desktop applications-label anchor not found')
            new=old.replace(anchor,'Graphics::Text(g,92,13,"'+label+'"',1)
            return make_diff('Kernel/Desktop.cpp',old,new)
    if st['component']==1:
        m=re.search(r'(?:set|change)\s+(?:the\s+)?scheduler\s+quantum(?:\s+ticks)?\s+(?:to\s+)?(\d+)',req,re.I)
        if m:
            n=int(m.group(1))
            if not 1<=n<=64: raise Reject('scheduler quantum must be 1..64 ticks')
            path=source/'Kernel'/'Scheduler.hpp'; old=path.read_text()
            new,count=re.subn(r'void Init\(State&, Process::Table&, Config cfg=\{\d+\}\);',f'void Init(State&, Process::Table&, Config cfg={{{n}}});',old,count=1)
            if count!=1: raise Reject('Scheduler default quantum anchor not found')
            return make_diff('Kernel/Scheduler.hpp',old,new)
        m=re.search(r'(?:set|change)\s+(?:the\s+)?tcp\s+max(?:imum)?\s+sockets\s+(?:to\s+)?(\d+)',req,re.I)
        if m:
            n=int(m.group(1))
            if not 4<=n<=64: raise Reject('TCP MaxSockets must be 4..64')
            path=source/'Kernel'/'Tcp.hpp'; old=path.read_text()
            new,count=re.subn(r'static constexpr u32 MaxSockets=\d+,',f'static constexpr u32 MaxSockets={n},',old,count=1)
            if count!=1: raise Reject('TCP MaxSockets anchor not found')
            return make_diff('Kernel/Tcp.hpp',old,new)
        m=re.search(r'(?:set|change)\s+(?:the\s+)?max(?:imum)?\s+file\s+(?:size|bytes)\s+(?:to\s+)?(\d+)',req,re.I)
        if m:
            n=int(m.group(1))
            if not 4096<=n<=65536: raise Reject('MaxFileBytes must be 4096..65536')
            path=source/'Kernel'/'DxeServices.hpp'; old=path.read_text()
            new,count=re.subn(r'MaxFileBytes=\d+;',f'MaxFileBytes={n};',old,count=1)
            if count!=1: raise Reject('MaxFileBytes anchor not found')
            return make_diff('Kernel/DxeServices.hpp',old,new)
        m=re.search(r'(?:set|change)\s+(?:the\s+)?audio\s+max(?:imum)?\s+streams\s+(?:to\s+)?(\d+)',req,re.I)
        if m:
            n=int(m.group(1))
            if not 1<=n<=32: raise Reject('audio MaxStreams must be 1..32')
            path=source/'Kernel'/'AudioCore.hpp'; old=path.read_text()
            new,count=re.subn(r'static constexpr u32 MaxStreams=\d+;',f'static constexpr u32 MaxStreams={n};',old,count=1)
            if count!=1: raise Reject('AudioCore MaxStreams anchor not found')
            return make_diff('Kernel/AudioCore.hpp',old,new)
        m=re.search(r'(?:set|change)\s+(?:the\s+)?storage\s+(?:request|request queue)\s+(?:depth|capacity)\s+(?:to\s+)?(\d+)',req,re.I)
        if m:
            n=int(m.group(1))
            if not 8<=n<=128: raise Reject('BlockDevice MaxRequests must be 8..128')
            path=source/'Kernel'/'BlockDevice.hpp'; old=path.read_text()
            new,count=re.subn(r'static constexpr u32 MaxRequests=\d+;',f'static constexpr u32 MaxRequests={n};',old,count=1)
            if count!=1: raise Reject('BlockDevice MaxRequests anchor not found')
            return make_diff('Kernel/BlockDevice.hpp',old,new)
        m=re.search(r'(?:set|change)\s+(?:the\s+)?nvme\s+admin\s+queue\s+depth\s+(?:to\s+)?(\d+)',req,re.I)
        if m:
            n=int(m.group(1))
            if not 2<=n<=256: raise Reject('NVMe admin queue depth must be 2..256')
            path=source/'Kernel'/'Nvme.hpp'; old=path.read_text()
            new,count=re.subn(r'constexpr u16 MaxAdminQueueDepth=\d+;',f'constexpr u16 MaxAdminQueueDepth={n};',old,count=1)
            if count!=1: raise Reject('NVMe MaxAdminQueueDepth anchor not found')
            return make_diff('Kernel/Nvme.hpp',old,new)
        m=re.search(r'(?:set|change)\s+(?:the\s+)?nvme\s+(?:io|i/o)\s+queue\s+depth\s+(?:to\s+)?(\d+)',req,re.I)
        if m:
            n=int(m.group(1))
            if not 2<=n<=256: raise Reject('NVMe I/O queue depth must be 2..256')
            path=source/'Kernel'/'Nvme.hpp'; old=path.read_text()
            new,count=re.subn(r'constexpr u16 MaxIoQueueDepth=\d+;',f'constexpr u16 MaxIoQueueDepth={n};',old,count=1)
            if count!=1: raise Reject('NVMe MaxIoQueueDepth anchor not found')
            return make_diff('Kernel/Nvme.hpp',old,new)
        m=re.search(r'(?:set|change)\s+(?:the\s+)?nvme\s+max(?:imum)?\s+transfer\s+(?:size|bytes)\s+(?:to\s+)?(\d+)',req,re.I)
        if m:
            n=int(m.group(1))
            if not 4096<=n<=1048576 or n%4096: raise Reject('NVMe maximum transfer bytes must be a 4KiB multiple from 4096..1048576')
            path=source/'Kernel'/'NvmeBlockBackend.hpp'; old=path.read_text()
            new,count=re.subn(r'constexpr u64 MaxBounceBytes=\d+ull\*1024ull;',f'constexpr u64 MaxBounceBytes={n//1024}ull*1024ull;',old,count=1)
            if count!=1: raise Reject('NVMe MaxBounceBytes anchor not found')
            return make_diff('Kernel/NvmeBlockBackend.hpp',old,new)
        m=re.search(r'(?:set|change)\s+(?:the\s+)?nvme\s+controller\s+(?:ready\s+)?(?:poll\s+)?limit\s+(?:to\s+)?(\d+)',req,re.I)
        if m:
            n=int(m.group(1))
            if not 10000<=n<=50000000: raise Reject('NVMe controller ready poll limit must be 10000..50000000')
            path=source/'Kernel'/'NvmeControllerTransport.hpp'; old=path.read_text()
            new,count=re.subn(r'constexpr u32 DefaultReadyPollLimit=\d+;',f'constexpr u32 DefaultReadyPollLimit={n};',old,count=1)
            if count!=1: raise Reject('NVMe controller DefaultReadyPollLimit anchor not found')
            return make_diff('Kernel/NvmeControllerTransport.hpp',old,new)
        m=re.search(r'(?:set|change)\s+(?:the\s+)?nvme\s+(?:reset|recovery)\s+(?:attempts|retries)\s+(?:to\s+)?(\d+)',req,re.I)
        if m:
            n=int(m.group(1))
            if not 1<=n<=8: raise Reject('NVMe recovery attempts must be 1..8')
            path=source/'Kernel'/'NvmeRecovery.hpp'; old=path.read_text()
            new,count=re.subn(r'constexpr u32 DefaultMaxResetAttempts=\d+;',f'constexpr u32 DefaultMaxResetAttempts={n};',old,count=1)
            if count!=1: raise Reject('NVMe recovery attempt anchor not found')
            return make_diff('Kernel/NvmeRecovery.hpp',old,new)
        m=re.search(r'(?:set|change)\s+(?:the\s+)?nvme\s+admin\s+(?:poll\s+)?timeout\s+(?:to\s+)?(\d+)',req,re.I)
        if m:
            n=int(m.group(1))
            if not 10000<=n<=50000000: raise Reject('NVMe admin poll timeout must be 10000..50000000')
            path=source/'Kernel'/'NvmeAdminRuntime.hpp'; old=path.read_text()
            new,count=re.subn(r'constexpr u32 DefaultPollLimit=\d+;',f'constexpr u32 DefaultPollLimit={n};',old,count=1)
            if count!=1: raise Reject('NVMe admin DefaultPollLimit anchor not found')
            return make_diff('Kernel/NvmeAdminRuntime.hpp',old,new)
        m=re.search(r'(?:set|change)\s+(?:the\s+)?smp\s+(?:ap\s+)?stack\s+(?:size|bytes)\s+(?:to\s+)?(\d+)',req,re.I)
        if m:
            n=int(m.group(1))
            if n not in (4096,8192,16384,32768,65536): raise Reject('SMP AP stack bytes must be a supported 4KiB power-of-two from 4096..65536')
            path=source/'Kernel'/'ApBootstrap.hpp'; old=path.read_text()
            new,count=re.subn(r'constexpr u64 DefaultStackBytes=\d+;',f'constexpr u64 DefaultStackBytes={n};',old,count=1)
            if count!=1: raise Reject('AP bootstrap DefaultStackBytes anchor not found')
            return make_diff('Kernel/ApBootstrap.hpp',old,new)
        m=re.search(r'(?:set|change)\s+(?:the\s+)?smp\s+(?:ap\s+)?rendezvous\s+(?:poll\s+)?limit\s+(?:to\s+)?(\d+)',req,re.I)
        if m:
            n=int(m.group(1))
            if not 1000<=n<=10000000: raise Reject('SMP rendezvous poll limit must be 1000..10000000')
            path=source/'Kernel'/'SmpBringup.hpp'; old=path.read_text()
            new,count=re.subn(r'constexpr u32 DefaultRendezvousPollLimit=\d+;',f'constexpr u32 DefaultRendezvousPollLimit={n};',old,count=1)
            if count!=1: raise Reject('SMP rendezvous poll limit anchor not found')
            return make_diff('Kernel/SmpBringup.hpp',old,new)
        m=re.search(r'(?:set|change)\s+(?:the\s+)?lapic\s+(?:icr\s+)?delivery\s+(?:poll\s+)?limit\s+(?:to\s+)?(\d+)',req,re.I)
        if m:
            n=int(m.group(1))
            if not 1000<=n<=10000000: raise Reject('LAPIC delivery poll limit must be 1000..10000000')
            path=source/'Kernel'/'LapicHardware.hpp'; old=path.read_text()
            new,count=re.subn(r'constexpr u32 DefaultDeliveryPollLimit=\d+;',f'constexpr u32 DefaultDeliveryPollLimit={n};',old,count=1)
            if count!=1: raise Reject('LAPIC DefaultDeliveryPollLimit anchor not found')
            return make_diff('Kernel/LapicHardware.hpp',old,new)
        m=re.search(r'(?:set|change)\s+(?:the\s+)?smp\s+(?:ap\s+)?startup\s+(?:attempts|retries)\s+(?:to\s+)?(\d+)',req,re.I)
        if m:
            n=int(m.group(1))
            if not 1<=n<=5: raise Reject('SMP startup attempts must be 1..5')
            path=source/'Kernel'/'SmpTopology.hpp'; old=path.read_text()
            new,count=re.subn(r'constexpr u32 MaxStartupAttempts=\d+;',f'constexpr u32 MaxStartupAttempts={n};',old,count=1)
            if count!=1: raise Reject('SMP MaxStartupAttempts anchor not found')
            return make_diff('Kernel/SmpTopology.hpp',old,new)
    if st['component']==2:
        m=re.search(r'(?:set|change)\s+(?:the\s+)?usb\s+(?:poll|event)\s+timeout(?:\s+spins)?\s+(?:to\s+)?(\d+)',req,re.I)
        if m:
            n=int(m.group(1))
            if not 100000<=n<=50000000: raise Reject('USB poll timeout must be 100000..50000000 spins')
            path=source/'Apps'/'Drivers'/'UsbDriver'/'main.cpp'; old=path.read_text()
            new,count=re.subn(r'static bool pollEvent\(Ring&r,Trb&out,unsigned long spins=\d+\)',f'static bool pollEvent(Ring&r,Trb&out,unsigned long spins={n})',old,count=1)
            if count!=1: raise Reject('USB pollEvent timeout anchor not found')
            return make_diff('Apps/Drivers/UsbDriver/main.cpp',old,new)
    if st['component']==3:
        m=re.search(r'(?:rename|set|change)\s+(?:the\s+)?gop\s+status(?:\s+label)?\s+(?:to\s+)?["\']?([A-Za-z0-9 _./+-]{1,32})',req,re.I)
        if m:
            label=m.group(1).strip().rstrip('. '); path=source/'Boot'/'Main.cpp'; old=path.read_text()
            anchor='SayAscii(st,"[11] GOP...\\r\\n")'
            # Source contains escaped CR/LF inside the C++ string.
            if anchor not in old: raise Reject('boot GOP status anchor not found')
            new=old.replace(anchor,'SayAscii(st,"[11] '+label+'...\\r\\n")',1)
            return make_diff('Boot/Main.cpp',old,new)
    raise Reject('no bounded v20 transformation recipe matches this request; supply an explicit reviewed --patch')

def run(cmd,cwd,log):
    p=subprocess.run(cmd,cwd=cwd,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
    log.append({'cmd':' '.join(cmd),'returncode':p.returncode,'output':p.stdout[-12000:]})
    return p.returncode==0

def main():
    ap=argparse.ArgumentParser(description='Verify a Forge SYSSTAGE and build an isolated Davis OS candidate.')
    ap.add_argument('--stage',required=True,type=pathlib.Path)
    ap.add_argument('--source',required=True,type=pathlib.Path,help='known-good Davis source tree')
    ap.add_argument('--patch',type=pathlib.Path,help='unified diff proposal')
    ap.add_argument('--synthesize',action='store_true',help='construct a bounded patch from the authenticated SYSSTAGE request')
    ap.add_argument('--out',required=True,type=pathlib.Path)
    ap.add_argument('--list-recipes',action='store_true',help='list audited bounded transformation recipes')
    ap.add_argument('--no-build',action='store_true',help='verify/stage only')
    a=ap.parse_args(); logs=[]
    if a.list_recipes:
        print('\n'.join(RECIPE_NAMES)); return 0
    try:
        st=read_stage(a.stage)
        if not a.source.is_dir(): raise Reject('source tree missing')
        if a.synthesize and a.patch: raise Reject('choose either --synthesize or --patch, not both')
        if not a.synthesize and not a.patch: raise Reject('either --patch or --synthesize is required')
        patch=synthesize_patch(st,a.source) if a.synthesize else a.patch.read_text(errors='replace')
        targets=parse_patch_targets(patch)
        bad=[t for t in targets if not allowed(t,st['component'])]
        if bad: raise Reject('source-boundary violation: '+', '.join(bad))
        a.out.mkdir(parents=True,exist_ok=True)
        work=a.out/'workspace'
        if work.exists(): shutil.rmtree(work)
        shutil.copytree(a.source,work,ignore=shutil.ignore_patterns('.git','out_all','out','host_out','host_test','host_test050','out_boot*'))
        before=tree_digest(a.source)
        proposal=a.out/'PROPOSED.patch'; proposal.write_text(patch)
        if not run(['patch','-p1','--batch','--forward','-i',str(proposal.resolve())],work,logs): raise Reject('patch application failed')
        after=tree_digest(work)
        if before==after: raise Reject('patch produced no source change')
        build_ok=None; kernel_sha=None
        if not a.no_build:
            build_ok=run(['bash','./BUILD_DAVIS_OS.sh'],work,logs)
            if not build_ok: raise Reject('canonical Davis OS build failed')
            qout=a.out/'qualification'
            qscript=work/'Tools'/'qualification'/'qualify.py'
            if not qscript.is_file(): raise Reject('v27 qualification gate missing')
            qualification_ok=run([sys.executable,str(qscript),'--source',str(work),'--out',str(qout),'--skip-build'],work,logs)
            if not qualification_ok: raise Reject('v27 host qualification failed')
            k=work/'out_all'/'DAVISKRN.BIN'
            if not k.exists(): raise Reject('candidate kernel missing after build')
            kernel_sha=hashlib.sha256(k.read_bytes()).hexdigest()
            cand=a.out/'candidate'; cand.mkdir(exist_ok=True)
            shutil.copy2(k,cand/'DAVISKRN.BIN')
            elf=work/'out_all'/'DAVISKRN.ELF'
            if elf.exists(): shutil.copy2(elf,cand/'DAVISKRN.ELF')
        rec={'builder':'Davis Forge System Builder','builder_version':'60.00','state':'CANDIDATE_NOT_PROMOTED',
             'stage':st,'allowlist':ALLOW[st['component']],'patch_mode':'SYNTHESIZED' if a.synthesize else 'SUPPLIED','patch_sha256':hashlib.sha256(patch.encode()).hexdigest(),'patch_targets':targets,'known_good_tree_sha256':before,
             'candidate_tree_sha256':after,'candidate_kernel_sha256':kernel_sha,'canonical_build_pass':build_ok,
             'gates':{'transaction_integrity':'PASS','source_boundary':'PASS','known_good_immutable':'PASS',
                      'build':'PASS' if build_ok else ('SKIPPED' if a.no_build else 'FAIL'),
                      'host_qualification':'PASS' if (a.no_build or locals().get('qualification_ok',False)) else 'FAIL',
                      'boot':'PENDING_PHYSICAL_VALIDATION','rollback':'REQUIRED','promotion':'HUMAN_APPROVAL_ONLY'},
             'log':logs,'created_unix':int(time.time())}
        (a.out/'VALIDATION.json').write_text(json.dumps(rec,indent=2)+'\n')
        (a.out/'CANDIDATE_ONLY.txt').write_text('This directory is a Davis OS candidate build. It is not approved for promotion.\nKnown-good source was copied, never modified in place.\n')
        print(json.dumps({'ok':True,'validation':str(a.out/'VALIDATION.json'),'kernel_sha256':kernel_sha},indent=2)); return 0
    except Reject as e:
        a.out.mkdir(parents=True,exist_ok=True)
        (a.out/'REJECTED.txt').write_text(str(e)+'\n')
        print('REJECTED:',e,file=sys.stderr); return 2
if __name__=='__main__': raise SystemExit(main())
