#!/usr/bin/env python3
"""Davis Forge Atomic System Plan Builder v21.
Combines multiple authenticated SYSSTAGE transactions into one disposable source
workspace, enforces each transaction's own source boundary, builds once, and emits
one all-or-nothing candidate. It has no promotion path.
"""
from __future__ import annotations
import argparse, hashlib, json, pathlib, shutil, subprocess, sys, time
import system_builder as sb

class Reject(RuntimeError): pass

def sha(p:pathlib.Path): return hashlib.sha256(p.read_bytes()).hexdigest()
def run(cmd,cwd,logs):
    p=subprocess.run(cmd,cwd=cwd,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
    logs.append({'cmd':' '.join(cmd),'returncode':p.returncode,'output':p.stdout[-12000:]})
    return p.returncode==0

def main():
    ap=argparse.ArgumentParser(description='Build one atomic Davis OS candidate from multiple Forge SYSSTAGE transactions.')
    ap.add_argument('--plan',required=True,type=pathlib.Path,help='JSON: {"stages":["a.SYSSTAGE",...]}')
    ap.add_argument('--source',required=True,type=pathlib.Path)
    ap.add_argument('--out',required=True,type=pathlib.Path)
    ap.add_argument('--no-build',action='store_true')
    a=ap.parse_args(); logs=[]
    try:
        if not a.source.is_dir(): raise Reject('known-good source tree missing')
        plan=json.loads(a.plan.read_text())
        entries=plan.get('stages',[])
        if not isinstance(entries,list) or not 2<=len(entries)<=8: raise Reject('plan must contain 2..8 stages')
        stage_paths=[]
        for x in entries:
            p=pathlib.Path(x); p=p if p.is_absolute() else (a.plan.parent/p)
            stage_paths.append(p.resolve())
        stages=[sb.read_stage(p) for p in stage_paths]
        # Duplicate authenticated requests add no value and can hide planner mistakes.
        keys=[(s['digest'],s['request_digest']) for s in stages]
        if len(set(keys))!=len(keys): raise Reject('duplicate SYSSTAGE transaction in plan')
        a.out.mkdir(parents=True,exist_ok=True)
        work=a.out/'workspace'
        if work.exists(): shutil.rmtree(work)
        shutil.copytree(a.source,work,ignore=shutil.ignore_patterns('.git','out_all','out','host_out','host_test','host_test050','out_boot*'))
        before=sb.tree_digest(a.source)
        tx=[]; touched=set(); combined=[]
        # Synthesize against the progressively modified disposable workspace so anchors
        # and conflicts are evaluated in transaction order.
        for i,(sp,st) in enumerate(zip(stage_paths,stages),1):
            patch=sb.synthesize_patch(st,work)
            targets=sb.parse_patch_targets(patch)
            bad=[t for t in targets if not sb.allowed(t,st['component'])]
            if bad: raise Reject('transaction %d source-boundary violation: %s'%(i,', '.join(bad)))
            pp=a.out/f'{i:02d}_PROPOSED.patch'; pp.write_text(patch)
            if not run(['patch','-p1','--batch','--forward','-i',str(pp.resolve())],work,logs): raise Reject(f'transaction {i} patch application failed')
            touched.update(targets); combined.append(f'# TRANSACTION {i}: {st["request"]}\n'+patch)
            tx.append({'index':i,'stage':str(sp),'stage_digest':st['digest'],'request':st['request'],'component':st['component_name'],'risk':st['risk'],'allowlist':sb.ALLOW[st['component']],'targets':targets,'patch_sha256':hashlib.sha256(patch.encode()).hexdigest()})
        after=sb.tree_digest(work)
        if before==after: raise Reject('atomic plan produced no source change')
        (a.out/'COMBINED_PROPOSAL.patch').write_text('\n'.join(combined))
        build_ok=None; kernel_sha=None
        if not a.no_build:
            build_ok=run(['bash','./BUILD_DAVIS_OS.sh'],work,logs)
            if not build_ok: raise Reject('canonical Davis OS build failed; entire plan rejected')
            k=work/'out_all'/'DAVISKRN.BIN'
            if not k.exists(): raise Reject('candidate kernel missing after build')
            kernel_sha=sha(k); cand=a.out/'candidate'; cand.mkdir(exist_ok=True)
            shutil.copy2(k,cand/'DAVISKRN.BIN')
            elf=work/'out_all'/'DAVISKRN.ELF'
            if elf.exists(): shutil.copy2(elf,cand/'DAVISKRN.ELF')
        rec={'builder':'Davis Forge Atomic System Plan Builder','builder_version':'21.00','state':'ATOMIC_CANDIDATE_NOT_PROMOTED','transaction_count':len(tx),'transactions':tx,'touched_files':sorted(touched),'known_good_tree_sha256':before,'candidate_tree_sha256':after,'candidate_kernel_sha256':kernel_sha,'canonical_build_pass':build_ok,'atomicity':'ALL_STAGES_ONE_WORKSPACE_ONE_BUILD_NO_PARTIAL_PROMOTION','gates':{'all_transaction_integrity':'PASS','all_source_boundaries':'PASS','known_good_immutable':'PASS','combined_build':'PASS' if build_ok else ('SKIPPED' if a.no_build else 'FAIL'),'boot':'PENDING_PHYSICAL_VALIDATION','rollback':'REQUIRED','promotion':'HUMAN_APPROVAL_ONLY'},'log':logs,'created_unix':int(time.time())}
        (a.out/'VALIDATION.json').write_text(json.dumps(rec,indent=2)+'\n')
        (a.out/'CANDIDATE_ONLY.txt').write_text('Atomic Davis OS candidate only. No transaction in this plan is promoted independently.\nKnown-good source was never modified in place.\n')
        print(json.dumps({'ok':True,'transactions':len(tx),'validation':str(a.out/'VALIDATION.json'),'kernel_sha256':kernel_sha},indent=2)); return 0
    except (Reject,sb.Reject,ValueError,json.JSONDecodeError) as e:
        a.out.mkdir(parents=True,exist_ok=True); (a.out/'REJECTED.txt').write_text(str(e)+'\n'); print('REJECTED:',e,file=sys.stderr); return 2
if __name__=='__main__': raise SystemExit(main())
