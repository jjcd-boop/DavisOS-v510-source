import pathlib, subprocess, tempfile, struct, sys
root=pathlib.Path(__file__).parent
with tempfile.TemporaryDirectory() as d:
 d=pathlib.Path(d); (d/'c.bin').write_bytes(b'\x90\xc3')
 subprocess.check_call([sys.executable,str(root/'dxe_pack.py'),'--code',str(d/'c.bin'),'-o',str(d/'a.dxe'),'--cap','file-read'])
 b=(d/'a.dxe').read_bytes(); assert struct.unpack_from('<I',b)[0]==0x32455844
 assert len(b)>2 and b[-2:]==b'\x90\xc3'
 print('PASS v1.06 dxe-pack image construction')
