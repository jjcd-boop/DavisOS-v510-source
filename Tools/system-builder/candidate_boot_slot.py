#!/usr/bin/env python3
"""Davis Forge Candidate Boot Slot Packager v24.
Creates a non-promoted one-shot candidate slot. Bootloader tries each stage digest once;
kernel health acknowledgement records success in UEFI NVRAM. Failure falls back next boot.
"""
import argparse, hashlib, json, pathlib, shutil, struct, sys
MAGIC=0x47424344

def main():
 ap=argparse.ArgumentParser();ap.add_argument('--validation',required=True,type=pathlib.Path);ap.add_argument('--candidate-kernel',required=True,type=pathlib.Path);ap.add_argument('--usb-root',required=True,type=pathlib.Path);a=ap.parse_args()
 v=json.loads(a.validation.read_text())
 if not v.get('canonical_build_pass'): raise SystemExit('REJECTED: candidate did not pass canonical build')
 if v.get('state') not in ('CANDIDATE_NOT_PROMOTED','ATOMIC_CANDIDATE_NOT_PROMOTED'): raise SystemExit('REJECTED: validation is not a non-promoted candidate')
 k=a.candidate_kernel.read_bytes();kh=hashlib.sha256(k).hexdigest()
 if v.get('candidate_kernel_sha256') and v['candidate_kernel_sha256']!=kh: raise SystemExit('REJECTED: candidate kernel digest mismatch')
 identity=(v.get('candidate_tree_sha256','')+'|'+kh+'|'+json.dumps(v.get('stage',v.get('transactions',[])),sort_keys=True)).encode()
 d=int.from_bytes(hashlib.sha256(identity).digest()[:4],'little') or 1
 slot=a.usb_root/'DAVIS'/'CANDIDATE';slot.mkdir(parents=True,exist_ok=True)
 (slot/'DAVISKRN.BIN').write_bytes(k);(slot/'CANDIDATE.STAGE').write_bytes(struct.pack('<II',MAGIC,d))
 rec={'format':'Davis Candidate Boot Slot','version':'24.00','stage_digest':d,'kernel_sha256':kh,'policy':'ONE_SHOT_TEST_THEN_KNOWN_GOOD','promotion':'NOT_IMPLEMENTED_HUMAN_APPROVAL_REQUIRED','health_ack':'UEFI_NVRAM_AFTER_RING3_STARTUP','checkpoint_record':'KERNEL_ENTRY>MEMORY>RUNTIME>HARDWARE>INPUT>DESKTOP>RING3>HEALTHY','failure_recovery':'NEXT_BOOT_KNOWN_GOOD'}
 (slot/'BOOT_SLOT.json').write_text(json.dumps(rec,indent=2)+'\n');print(json.dumps(rec,indent=2))
if __name__=='__main__': main()
