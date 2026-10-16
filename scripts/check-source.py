"""Audit the public source tree without reading ignored local runtime data."""
from pathlib import Path
import hashlib,json,re,sys
root=Path(__file__).resolve().parents[1]
allowed={'.c','.h','.py','.ps1','.md','.json','.txt','.yml','.yaml'}
ignored={'.git','build','runtime','.cache','.venv','__pycache__'}
count=0
for p in root.rglob('*'):
 parts=p.relative_to(root).parts
 if not p.is_file() or parts[0] in ignored or '__pycache__' in parts:continue
 rel=p.relative_to(root).as_posix()
 assert parts[0] in {'src','tests','scripts','tools','docs','.github'} or len(parts)==1 and p.name in {'.gitignore','.gitattributes','README.md','READMD.md','CONTRIBUTING.md','LICENSE','NOTICE','SECURITY.md'},('Unexpected public path',rel)
 assert p.name in {'.gitignore','.gitattributes'} or p.suffix in allowed,rel
 assert p.stat().st_size<1_000_000,('Unexpected large payload',rel)
 text=p.read_text(encoding='utf-8-sig')
 assert not re.search(r'[A-Za-z]:[\\/](?:Users|BaiduNetdiskDownload)[\\/]',text,re.I),('Personal machine path',rel)
 assert not re.search(r'gh[pousr]_[A-Za-z0-9]{30,}|github_pat_[A-Za-z0-9_]{30,}',text),('Possible token',rel)
 count+=1
manifest=json.loads((root/'docs/source-manifest.json').read_text())
for row in manifest['files']:
 p=root/row['path'];assert hashlib.sha256(p.read_bytes()).hexdigest()==row['sha256'],row['path']
print('PASS source audit:',count,'text files; no runtime assets, binaries or personal paths')

