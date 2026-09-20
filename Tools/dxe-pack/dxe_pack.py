#!/usr/bin/env python3
import argparse, struct, pathlib, sys
MAGIC=0x32455844; USER=1<<1; PIE=1<<0
CAP={'window':1<<0,'file-read':1<<1,'file-write':1<<2,'network':1<<3,'ipc':1<<4}
SEG_R,SEG_W,SEG_X=1,2,4
HEADER_FMT='<IHHHHI' + 'Q'*12 + 'QIIQIIQIIQQ' + 'Q'*4
SEG_FMT='<QQQQIIQ'

def align(v,a): return (v+a-1)&~(a-1)
def main():
 p=argparse.ArgumentParser(description='Pack flat Davis application code/data into a DXE2 image')
 p.add_argument('--code',required=True); p.add_argument('--data'); p.add_argument('-o','--output',required=True)
 p.add_argument('--entry-offset',type=lambda x:int(x,0),default=0); p.add_argument('--code-va',type=lambda x:int(x,0),default=0x40000000)
 p.add_argument('--data-va',type=lambda x:int(x,0),default=0x50000000); p.add_argument('--bss',type=lambda x:int(x,0),default=0)
 p.add_argument('--stack',type=lambda x:int(x,0),default=65536); p.add_argument('--app-id',type=lambda x:int(x,0),default=1)
 p.add_argument('--version',default='1.0'); p.add_argument('--cap',action='append',default=[],choices=CAP)
 a=p.parse_args(); code=pathlib.Path(a.code).read_bytes(); data=pathlib.Path(a.data).read_bytes() if a.data else b''
 if not code: p.error('code must not be empty')
 try: maj,minr=(int(x) for x in a.version.split('.',1))
 except: p.error('--version must be MAJOR.MINOR')
 caps=0
 for c in a.cap:caps|=CAP[c]
 hsz=align(struct.calcsize(HEADER_FMT),16); nseg=1+(1 if data or a.bss else 0); segsz=struct.calcsize(SEG_FMT); table_off=hsz
 payload=align(table_off+nseg*segsz,16); code_off=payload; data_off=align(code_off+len(code),16) if (data or a.bss) else 0
 total=(data_off+len(data)) if data_off else (code_off+len(code)); entry=a.code_va+a.entry_offset
 if a.entry_offset<0 or a.entry_offset>=len(code): p.error('entry offset must point inside code')
 # Common DXE header prefix then DXE2 extension. DXE2 entryOffset is a virtual entry address.
 vals=[MAGIC,hsz,2,0,0,PIE|USER,total,entry,0,0,0,0,0,a.stack,0,0,0,0,a.app_id,maj,minr,caps,nseg,segsz,table_off,0,0,0,0,0,0,0,0]
 hdr=struct.pack(HEADER_FMT,*vals)
 hdr+=bytes(hsz-len(hdr)); segs=[]
 segs.append(struct.pack(SEG_FMT,code_off,len(code),a.code_va,len(code),SEG_R|SEG_X,4096,0))
 if data or a.bss: segs.append(struct.pack(SEG_FMT,data_off,len(data),a.data_va,len(data)+a.bss,SEG_R|SEG_W,4096,0))
 out=bytearray(hdr+b''.join(segs)); out+=bytes(code_off-len(out)); out+=code
 if data_off: out+=bytes(data_off-len(out)); out+=data
 if len(out)!=total: raise SystemExit('internal size mismatch')
 pathlib.Path(a.output).write_bytes(out)
 print(f'Packed {a.output}: {len(out)} bytes, {nseg} segment(s), entry=0x{entry:x}, caps=0x{caps:x}')
if __name__=='__main__': main()
