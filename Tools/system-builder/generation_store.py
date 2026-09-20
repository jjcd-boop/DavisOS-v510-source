#!/usr/bin/env python3
"""Davis OS Generation Store v25.
Creates/verifies/restores complete system-generation snapshots without touching /Home.
Restore always writes to a staging directory; it never overwrites a live USB root.
"""
import argparse, hashlib, json, pathlib, shutil, sys, time
FORMAT='DAVIS_GENERATION_V1'
SYSTEM_PATHS=('EFI/BOOT/BOOTX64.EFI','DAVISKRN.BIN','DAVIS/DAVISKRN.BIN','DAVIS/APPS')
EXCLUDE_TOP={'CANDIDATE','RECOVERY','HOME'}

def sha(p):
 h=hashlib.sha256()
 with p.open('rb') as f:
  for b in iter(lambda:f.read(1024*1024),b''): h.update(b)
 return h.hexdigest()
def files_under(root):
 out=[]
 for rel in SYSTEM_PATHS:
  p=root/rel
  if p.is_file(): out.append(p)
  elif p.is_dir(): out += [x for x in p.rglob('*') if x.is_file()]
 return sorted(set(out))
def relsafe(p,root): return p.relative_to(root).as_posix()
def snapshot(src,dst,label,role):
 if not (src/'DAVIS/DAVISKRN.BIN').is_file(): raise SystemExit('REJECTED: source is not a Davis system root')
 if dst.exists(): raise SystemExit('REJECTED: destination generation already exists')
 dst.mkdir(parents=True)
 entries=[]
 for p in files_under(src):
  rel=relsafe(p,src)
  # Never capture user data or transient candidate/recovery trees.
  parts=pathlib.PurePosixPath(rel).parts
  if len(parts)>1 and parts[0]=='DAVIS' and parts[1].upper() in EXCLUDE_TOP: continue
  q=dst/'SYSTEM'/rel;q.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(p,q)
  entries.append({'path':rel,'bytes':q.stat().st_size,'sha256':sha(q)})
 manifest={'format':FORMAT,'version':'25.00','label':label,'role':role,'created_unix':int(time.time()),'personal_data':'EXCLUDED','candidate_data':'EXCLUDED','file_count':len(entries),'files':entries}
 raw=json.dumps(manifest,sort_keys=True,separators=(',',':')).encode();manifest['manifest_sha256']=hashlib.sha256(raw).hexdigest()
 (dst/'MANIFEST.json').write_text(json.dumps(manifest,indent=2)+'\n')
 verify(dst); return manifest
def verify(gen):
 m=json.loads((gen/'MANIFEST.json').read_text()); claimed=m.pop('manifest_sha256',None)
 raw=json.dumps(m,sort_keys=True,separators=(',',':')).encode()
 if hashlib.sha256(raw).hexdigest()!=claimed: raise SystemExit('REJECTED: generation manifest integrity mismatch')
 if m.get('format')!=FORMAT: raise SystemExit('REJECTED: unsupported generation format')
 for e in m['files']:
  rel=pathlib.PurePosixPath(e['path'])
  if rel.is_absolute() or '..' in rel.parts: raise SystemExit('REJECTED: unsafe generation path')
  p=gen/'SYSTEM'/pathlib.Path(*rel.parts)
  if not p.is_file() or p.stat().st_size!=e['bytes'] or sha(p)!=e['sha256']: raise SystemExit('REJECTED: generation file mismatch: '+e['path'])
 return m,claimed
def restore(gen,out):
 m,d=verify(gen)
 if out.exists(): raise SystemExit('REJECTED: restore staging destination already exists')
 shutil.copytree(gen/'SYSTEM',out)
 (out/'RESTORE_VALIDATION.json').write_text(json.dumps({'state':'STAGED_NOT_PROMOTED','source_manifest_sha256':d,'label':m['label'],'role':m['role'],'operator_promotion_required':True},indent=2)+'\n')
 return m
def main():
 ap=argparse.ArgumentParser();sp=ap.add_subparsers(dest='cmd',required=True)
 s=sp.add_parser('snapshot');s.add_argument('--source',type=pathlib.Path,required=True);s.add_argument('--generation',type=pathlib.Path,required=True);s.add_argument('--label',required=True);s.add_argument('--role',choices=['FACTORY','KNOWN_GOOD','PREVIOUS'],required=True)
 v=sp.add_parser('verify');v.add_argument('--generation',type=pathlib.Path,required=True)
 r=sp.add_parser('restore');r.add_argument('--generation',type=pathlib.Path,required=True);r.add_argument('--out',type=pathlib.Path,required=True)
 a=ap.parse_args()
 if a.cmd=='snapshot': m=snapshot(a.source,a.generation,a.label,a.role);print(json.dumps({'state':'SNAPSHOT_VERIFIED','files':m['file_count'],'role':m['role']},indent=2))
 elif a.cmd=='verify': m,d=verify(a.generation);print(json.dumps({'state':'GENERATION_VERIFIED','files':m['file_count'],'manifest_sha256':d},indent=2))
 else: m=restore(a.generation,a.out);print(json.dumps({'state':'RESTORE_STAGED_NOT_PROMOTED','label':m['label']},indent=2))
if __name__=='__main__': main()
