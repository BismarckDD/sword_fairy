"""Back up and install one validated native-context map without resampling."""
import argparse
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import runpy
import shutil
import numpy as np
from PIL import Image

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--scene', required=True)
parser.add_argument('--variant', choices=('context', 'shared'), default='context')
args = parser.parse_args()
scene = args.scene.upper()
assert scene.isascii() and scene.isalnum()
root = Path(__file__).resolve().parents[1]
game = root/'runtime/game'
hd = game/'hd'
art = hd/'maps/imagegen'
work = hd/f'maps/context-tiles/{scene}-320'
source = work/('reconstructed-shared-tiles-4x.png' if args.variant == 'shared' else 'context-redraw-4x.png')
read = lambda p: json.loads(p.read_text(encoding='utf-8-sig'))
sha = lambda p: hashlib.sha256(p.read_bytes()).hexdigest()
validation, plan, report = [read(work/n) for n in ('validation.json','plan.json','report.json')]
assert validation['status'] == 'passed'
assert sha(source) == report['output_sha256'][source.name]
width, height = plan['source_size']
with Image.open(source) as im:
    assert im.mode in ('RGB', 'RGBA') and im.size == (width*4,height*4)
    if im.mode == 'RGBA':
        assert im.getextrema()[3] == (255, 255)
    original = np.asarray(im.convert('RGB'))
for name, expected in plan['archive_hashes'].items():
    assert sha(game/name) == expected
module = runpy.run_path(str(root/'tools/import-imagegen-maps.py'))
manifest = art/'scene-manifest.tsv'
metadata = [manifest,hd/'manifest.tsv',art/f'scene-verification/{scene}.json',art/'installation.json']
backup = art/'backups'/f'before-{scene}-context320-{datetime.now(timezone.utc):%Y%m%dT%H%M%S%fZ}'
with module['registration_lock']():
    old = manifest.read_text(encoding='utf-8').splitlines()
    rows = [r.split('\t') for r in old if r and not r.startswith('#')]
    others = [r for r in old if r and not r.startswith('#') and r.split('\t')[0] != scene]
    paths = set(metadata)
    for row in rows:
        if row[0] == scene:
            paths.update((hd/row[5],hd/row[6]))
    backup.mkdir(parents=True)
    existed = {path for path in metadata if path.exists()}
    hashes = {}
    for path in paths:
        if not path.exists():
            continue
        relative = path.relative_to(hd)
        dest = backup/'hd'/relative
        dest.parent.mkdir(parents=True,exist_ok=True)
        shutil.copy2(path,dest)
        hashes[relative.as_posix()] = sha(path)
        assert sha(dest) == sha(path)
    (backup/'backup.json').write_text(json.dumps(hashes,indent=2),encoding='utf-8')
    try:
        # Position overrides require original tile identities. Register missing
        # identities using original pixel art, so other scenes retain their art.
        base = hd/'manifest.tsv'
        before = base.read_text(encoding='utf-8')
        keys = {line.split('\t')[0] for line in before.splitlines() if line and not line.startswith('#')}
        anchor_rows = []
        folder = art/f'tile-anchors/{scene}'
        folder.mkdir(parents=True,exist_ok=True)
        for reference in sorted((work/'candidate-pack/references').glob('*.idx')):
            raw = reference.read_bytes()
            assert len(raw) == 1024
            if not any(value != 255 for value in raw[768:]):
                continue
            key = 'TILE_'+hashlib.sha256(raw[768:]).hexdigest()[:24]
            if key in keys:
                continue
            ref, image = folder/(key+'.idx'), folder/(key+'.bmp')
            ref.write_bytes(raw)
            palette = np.minimum(np.frombuffer(raw[:768],np.uint8).astype(np.uint16)*4,255).astype(np.uint8).reshape(256,3)
            indices = np.frombuffer(raw[768:],np.uint8).reshape(16,16)
            rgb = palette[indices].repeat(4,0).repeat(4,1)
            Image.fromarray(np.dstack((rgb,np.full((64,64),255,np.uint8)))).save(image)
            anchor_rows.append(f'{key}\t16\t16\t{ref.relative_to(hd).as_posix()}\t{image.relative_to(hd).as_posix()}')
            keys.add(key)
        if anchor_rows:
            assert base.read_text(encoding='utf-8') == before
            temporary = base.with_suffix('.context.tmp')
            temporary.write_text(before.rstrip('\r\n')+'\n'+'\n'.join(anchor_rows)+'\n',encoding='utf-8')
            temporary.replace(base)
        module['_scene_chunks'](scene,source)
        module['verify_scenes'](scene)
        current = manifest.read_text(encoding='utf-8').splitlines()
        assert [r for r in current if r and not r.startswith('#') and r.split('\t')[0]!=scene] == others
        active = [r.split('\t') for r in current if r and not r.startswith('#') and r.split('\t')[0]==scene]
        coverage = np.zeros((2,height//16,width//16),bool)
        checked = set()
        for name,x,y,w,h,reference,image in active:
            x,y,w,h = map(int,(x,y,w,h))
            layer = int(Path(reference).stem[len(name)+1:].split('_')[0])
            coverage[layer,y//16:(y+h)//16,x//16:(x+w)//16] = True
            if image not in checked:
                with Image.open(hd/image) as im:
                    assert np.array_equal(np.asarray(im.convert('RGB')),original[y*4:(y+h)*4,x*4:(x+w)*4])
                checked.add(image)
        assert coverage.all()
        result = dict(status='installed-and-resource-verified',scene=scene,variant=args.variant,
                      source=str(source),source_sha256=sha(source),backup=str(backup),
                      native_AI_scales=validation['native_AI_scales'],scene_records=len(active),
                      all_pixels_covered_both_layers=True,runtime_pixels_exact=True,
                      original_tile_anchors_added=len(anchor_rows),
                      anchor_art='Original nearest-neighbor pixels, palette-aware; enables position retagging',
                      resampling_during_import=False,other_scenes_unchanged=True,
                      original_archives_unchanged=True,native_probe_completed=False)
        dest = art/f'verification-reports/{scene}-context320-installation.json'
        dest.parent.mkdir(exist_ok=True)
        dest.write_text(json.dumps(result,indent=2),encoding='utf-8')
        print(json.dumps(result))
    except BaseException:
        for path in metadata:
            saved = backup/'hd'/path.relative_to(hd)
            if saved.exists():
                shutil.copy2(saved,path)
            elif path.exists() and path not in existed:
                path.unlink()
        raise
