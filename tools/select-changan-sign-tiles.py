"""Select the inspected, corrected native candidates for shared Chang'an signage."""
import hashlib
import json
from pathlib import Path
import numpy as np
from PIL import Image

Image.MAX_IMAGE_PIXELS = None  # Trusted local map dimensions are checked below.

root = Path(__file__).resolve().parents[1]
work = root/'runtime/game/hd/maps/context-tiles/CA00-320'
read = lambda p: json.loads(p.read_text(encoding='utf-8-sig'))
sha = lambda p: hashlib.sha256(p.read_bytes()).hexdigest()
report = read(work/'report.json')
catalog = read(root/'runtime/game/hd/maps/catalog.json')
scene = next(s for s in catalog['scenes'] if s['name'] == 'CA00')
ids = np.array(scene['indices']).reshape(2, scene['height'], scene['width'])
with Image.open(work/'context-redraw-4x.png') as image:
    assert image.size == (scene['width']*64, scene['height']*64)
    full = np.asarray(image.convert('RGB'))
selection = []
for x, expected in ((78,269),(79,271),(80,270),(81,272)):
    y = 131
    tile = int(ids[1,y,x])
    assert tile == expected
    record = report['records'][tile]
    assert record['tile'] == tile and record['status'] == 'recovered'
    path = work/'tiles'/f'{tile:04d}.png'
    with Image.open(path) as image:
        previous = np.array(image.convert('RGBA'))
    rgba = np.dstack((full[y*64:(y+1)*64,x*64:(x+1)*64], previous[:,:,3]))
    selection.append(dict(tile=tile, previous_source=record['source'], selected_source=[x*16,y*16,1],
                          reason='Inspected corrected traditional glyphs 安 and 長; actual candidate, no blending'))
    record.update(source=[x*16,y*16,1], manual_candidate_selection=True)
    Image.fromarray(rgba).save(path)
    Image.fromarray(rgba).save(work/'candidate-pack/images'/f'CA00_TILE_{tile:04d}.bmp')

library = {}
atlas = Image.new('RGBA',(1024,((report['library_tiles']+15)//16)*64))
for tile in range(report['library_tiles']):
    with Image.open(work/'tiles'/f'{tile:04d}.png') as image:
        library[tile] = image.convert('RGBA')
    atlas.paste(library[tile],((tile%16)*64,(tile//16)*64))
atlas.save(work/'replacement-library-64.png')
assembled = Image.new('RGBA',(scene['width']*64,scene['height']*64))
for layer in (0,1):
    for y in range(scene['height']):
        for x in range(scene['width']):
            tile = int(ids[layer,y,x])
            if layer and tile == 1:
                continue
            assembled.alpha_composite(library[tile],(x*64,y*64))
assembled.convert('RGB').save(work/'reconstructed-shared-tiles-4x.png')
rgb = np.asarray(assembled.convert('RGB'))
report['tile_boundary_rgb_jump']['shared_library'] = float(np.mean([
    np.abs(rgb[:,64::64].astype(np.int16)-rgb[:,63:-1:64].astype(np.int16)).mean(),
    np.abs(rgb[64::64].astype(np.int16)-rgb[63:-1:64].astype(np.int16)).mean()]))
report['shared_vs_context_rgb_mae'] = sum(float(np.abs(full[y:y+64].astype(np.int16)-rgb[y:y+64].astype(np.int16)).sum()) for y in range(0,len(full),64))/full.size
for name in report['output_sha256']:
    report['output_sha256'][name] = sha(work/name)
report['manual_candidate_selection'] = selection
(work/'report.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
(work/'manual-candidate-selection.json').write_text(json.dumps(dict(scene='CA00', selections=selection,
    source_sha256=sha(work/'context-redraw-4x.png'), selected_opaque_pixels_exact=True),indent=2),encoding='utf-8')
print(json.dumps(selection))
