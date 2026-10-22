"""Add original-pixel tile recognizers needed by position-specific map artwork."""
from pathlib import Path
import argparse, hashlib, json, shutil, struct
import numpy as np

ROOT=Path(__file__).resolve().parents[1]
HD=ROOT/'runtime/game/hd'

def install(scene,original_only=False):
    work=HD/'maps/context-tiles'/f'{scene}-320'
    if original_only:
        game=HD.parent
        catalog=json.loads((HD/'maps/catalog.json').read_text(encoding='utf-8-sig'))
        record=next(s for s in catalog['scenes'] if s['name']==scene and s['status']=='ready')
        def offsets(name):
            data=(game/f'{name}.IDX').read_bytes()
            return {data[i:i+12].split(b'\0')[0].decode().strip():struct.unpack_from('<I',data,i+12)[0] for i in range(0,len(data),16)}
        data=(game/'BLK.DAT').read_bytes();at=offsets('BLK')[record['block']]
        count,w,h=struct.unpack_from('<HHH',data,at);assert (w,h)==(16,16)
        tiles=np.frombuffer(data[at+6:at+6+count*256],np.uint8).reshape(count,4,16,4).transpose(0,2,3,1).reshape(count,16,16)
        palette=bytearray((game/'COMMON.PAL').read_bytes())
        maps=(game/'MAP.DAT').read_bytes();p=offsets('MAP')[record['palette']]
        palette[480:765]=maps[p+502:p+787]
        references=[(f'{t:04d}',bytes(palette)+tile.tobytes()) for t,tile in enumerate(tiles)]
        work=HD/'maps/context-tiles/reference-only'/scene
        work.mkdir(parents=True,exist_ok=True)
    else:
        validation=json.loads((work/'validation.json').read_text())
        assert validation['status']=='passed'
        references=[(ref.stem.rsplit('_',1)[1],ref.read_bytes()) for ref in sorted((work/'candidate-pack/references').glob('*.idx'))]
    manifest=HD/'manifest.tsv'
    data=manifest.read_bytes()
    backup=HD/'maps/imagegen/backups/chengdu-context-20261021'
    backup.mkdir(parents=True,exist_ok=True)
    digest=hashlib.sha256(data).hexdigest()
    dest=backup/f'main-manifest-{digest[:16]}.tsv'
    if not dest.exists(): dest.write_bytes(data)
    folder=HD/'imports/chengdu-context'/scene
    folder.mkdir(parents=True,exist_ok=True)
    prefix=f'TILE_CONTEXT_{scene}_'
    rows=[]
    for tile,raw in references:
        assert len(raw)==1024
        target=folder/f'{tile}.idx'
        target.write_bytes(raw)
        palette=np.frombuffer(raw[:768],np.uint8).astype(np.uint16)*4
        palette=np.minimum(palette,255).astype(np.uint8).reshape(256,3)
        indices=np.frombuffer(raw[768:],np.uint8).reshape(16,16)
        rgb=palette[indices].repeat(4,0).repeat(4,1)
        bgra=np.empty((64,64,4),np.uint8)
        bgra[:,:,:3]=rgb[:,:,::-1]; bgra[:,:,3]=255
        payload=bgra.tobytes()
        bmp=b'BM'+struct.pack('<III',54+len(payload),0,54)+struct.pack('<IiiHHIIIIII',40,64,-64,1,32,0,len(payload),0,0,0,0)+payload
        image=folder/f'{tile}.bmp'; image.write_bytes(bmp)
        rows.append(f'{prefix}{tile}\t16\t16\t{target.relative_to(HD).as_posix()}\t{image.relative_to(HD).as_posix()}')
    previous=[line for line in data.decode('utf-8-sig').splitlines() if not line.startswith(prefix)]
    updated='\n'.join(previous+rows)+'\n'
    assert manifest.read_bytes()==data, 'Manifest changed concurrently; retry'
    temporary=manifest.with_suffix('.context.tmp')
    temporary.write_text(updated,encoding='utf-8'); temporary.replace(manifest)
    report=dict(scene=scene,recognizers=len(rows),fallback='exact original nearest-neighbor 4x; generated scene artwork has priority',
                manifest_before_sha256=digest,manifest_after_sha256=hashlib.sha256(manifest.read_bytes()).hexdigest(),
                backup=str(dest),engine_modified=False)
    (work/'reference-installation.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
    print(json.dumps(report))

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('scenes',nargs='+');p.add_argument('--original-only',action='store_true');a=p.parse_args()
    for scene in a.scenes:
        assert scene.isalnum() and scene.isascii()
        install(scene.upper(),a.original_only)
