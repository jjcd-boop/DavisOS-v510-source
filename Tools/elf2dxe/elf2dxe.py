#!/usr/bin/env python3
import argparse, pathlib, struct, sys
ELF_MAGIC=b'\x7fELF'; PT_LOAD=1; PF_X=1; PF_W=2; PF_R=4
DXE_MAGIC=0x32455844; PIE=1; USER=2; R=1; W=2; X=4
CAP={'window':1<<0,'file-read':1<<1,'file-write':1<<2,'network':1<<3,'ipc':1<<4,'service-host':1<<16,'driver-io':1<<17,'irq':1<<18,'dma':1<<19}
HEADER_FMT='<IHHHHI' + 'Q'*12 + 'QIIQIIQIIQQ' + 'Q'*4
SEG_FMT='<QQQQIIQ'
def al(v,a): return (v+a-1)&~(a-1)
def die(s): raise SystemExit('elf2dxe: '+s)
def parse_elf(b):
 if len(b)<64 or b[:4]!=ELF_MAGIC: die('not an ELF file')
 if b[4]!=2 or b[5]!=1: die('requires ELF64 little-endian')
 e=struct.unpack_from('<16sHHIQQQIHHHHHH',b,0)
 _,etype,machine,ver,entry,phoff,shoff,flags,ehsize,phentsz,phnum,*_=e
 if machine!=62: die('requires x86-64 ELF (EM_X86_64)')
 if phentsz<56: die('invalid program-header size')
 seg=[]
 for i in range(phnum):
  o=phoff+i*phentsz
  if o+56>len(b): die('truncated program header table')
  typ,fl,off,va,pa,filesz,memsz,align=struct.unpack_from('<IIQQQQQQ',b,o)
  if typ!=PT_LOAD or memsz==0: continue
  if off+filesz>len(b) or filesz>memsz: die('invalid PT_LOAD bounds')
  if va&0xfff: die(f'PT_LOAD virtual address 0x{va:x} is not page aligned')
  sf=0
  if fl&PF_R: sf|=R
  if fl&PF_W: sf|=W
  if fl&PF_X: sf|=X
  if not sf&R: sf|=R
  if sf&(W|X)==(W|X): die('W+X PT_LOAD rejected')
  seg.append((off,filesz,va,memsz,sf,align or 4096))
 if not seg: die('no loadable segments')
 if len(seg)>8: die('more than 8 PT_LOAD segments')
 if not any((s[4]&X) and s[2]<=entry<s[2]+s[3] for s in seg): die('ELF entry is not inside executable PT_LOAD')
 return entry,seg

def main():
 p=argparse.ArgumentParser(description='Convert a linked x86-64 ELF application to Davis DXE2')
 p.add_argument('elf'); p.add_argument('-o','--output',required=True); p.add_argument('--app-id',type=lambda x:int(x,0),required=True)
 p.add_argument('--version',default='1.0'); p.add_argument('--stack',type=lambda x:int(x,0),default=65536); p.add_argument('--cap',action='append',default=[],choices=CAP)
 a=p.parse_args(); eb=pathlib.Path(a.elf).read_bytes(); entry,segs=parse_elf(eb)
 try: maj,minr=(int(x) for x in a.version.split('.',1))
 except: p.error('--version must be MAJOR.MINOR')
 caps=0
 for c in a.cap:caps|=CAP[c]
 hsz=al(struct.calcsize(HEADER_FMT),16); tableoff=hsz; segsz=struct.calcsize(SEG_FMT); payload=al(tableoff+len(segs)*segsz,16)
 # Repack each PT_LOAD payload contiguously. BSS remains represented by memoryBytes > fileBytes.
 desc=[]; blobs=[]; cur=payload
 for off,fs,va,ms,sf,aa in segs:
  cur=al(cur,16); desc.append((cur,fs,va,ms,sf,min(max(aa,1),4096),0)); blobs.append((cur,eb[off:off+fs])); cur+=fs
 total=cur
 vals=[DXE_MAGIC,hsz,2,0,0,PIE|USER,total,entry,0,0,0,0,0,a.stack,0,0,0,0,a.app_id,maj,minr,caps,len(desc),segsz,tableoff,0,0,0,0,0,0,0,0]
 hdr=struct.pack(HEADER_FMT,*vals)+bytes(hsz-struct.calcsize(HEADER_FMT))
 out=bytearray(hdr+b''.join(struct.pack(SEG_FMT,*d) for d in desc))
 for off,blob in blobs:
  out+=bytes(off-len(out)); out+=blob
 if len(out)!=total: die('internal output size mismatch')
 pathlib.Path(a.output).write_bytes(out)
 print(f'ELF64 -> DXE2: {a.output} ({len(out)} bytes, {len(desc)} segments, entry=0x{entry:x}, caps=0x{caps:x})')
if __name__=='__main__': main()
