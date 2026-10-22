"""Non-destructive 320/640px context redraw -> shared 64px tiles.

prepare exports exact source crops; strict finish requires native 2560px art.
Explicit diagnostic mode permits resampling but never marks it as accepted.
Original archives stay read-only.
"""
import argparse
import hashlib
import json
import struct
from pathlib import Path
import numpy as np
from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
GAME = ROOT / 'runtime/game'
WORK = GAME / 'hd/maps/context-tiles/HENS00'
sha = lambda b: hashlib.sha256(b).hexdigest()


def save(name, obj):
    (WORK/name).write_text(json.dumps(obj, ensure_ascii=False, indent=2), encoding='utf-8')


def inputs():
    catalog = json.loads((GAME/'hd/maps/catalog.json').read_text(encoding='utf-8-sig'))
    scene = next(s for s in catalog['scenes'] if s['name'] == 'HENS00')
    def index(name):
        b = (GAME/(name+'.IDX')).read_bytes()
        return {b[i:i+12].split(b'\0')[0].decode().strip(): struct.unpack_from('<I', b, i+12)[0] for i in range(0,len(b),16)}
    blocks = (GAME/'BLK.DAT').read_bytes()
    at = index('BLK')[scene['block']]
    count = struct.unpack_from('<H', blocks, at)[0]
    tiles = np.frombuffer(blocks[at+6:at+6+count*256], np.uint8).reshape(count,4,16,4).transpose(0,2,3,1).reshape(count,16,16)
    maps = (GAME/'MAP.DAT').read_bytes()
    pal = bytearray((GAME/'COMMON.PAL').read_bytes())
    p = index('MAP')[scene['palette']]
    pal[480:765] = maps[p+502:p+787]
    rgb = np.minimum(np.frombuffer(pal,np.uint8).reshape(256,3).astype(np.uint16)*4,255).astype(np.uint8)
    ids = np.array(scene['indices']).reshape(2,scene['height'],scene['width'])
    return scene,tiles,rgb,ids


def prepare(chunk_size=640):
    WORK.mkdir(parents=True, exist_ok=True)
    for name in ('source','generated','tiles','previews'):
        (WORK/name).mkdir(exist_ok=True)
    scene,tiles,pal,ids = inputs()
    original = Image.new('RGBA',(scene['width']*16,scene['height']*16))
    for layer in (0,1):
        for y in range(scene['height']):
            for x in range(scene['width']):
                t = int(ids[layer,y,x])
                if layer and t == 1: continue
                a = np.dstack((pal[tiles[t]],(tiles[t]!=255).astype(np.uint8)*255))
                original.alpha_composite(Image.fromarray(a),(x*16,y*16))
    expected = Image.open(GAME/'hd/maps/source/HENS00.png').convert('RGBA')
    assert np.array_equal(np.asarray(original),np.asarray(expected)), 'Original tile decoder roundtrip differs'
    im = Image.open(GAME/'hd/maps/source/HENS00.png').convert('RGB')
    jobs = []
    # Shift the final window inward: actual map context beats invented padding.
    ys = sorted({min(y,max(0,im.height-chunk_size)) for y in range(0,im.height,chunk_size)})
    xs = sorted({min(x,max(0,im.width-chunk_size)) for x in range(0,im.width,chunk_size)})
    for y in ys:
        for x in xs:
            w,h = min(chunk_size,im.width-x),min(chunk_size,im.height-y)
            a = np.asarray(im)[y:y+h,x:x+w]
            padded = np.pad(a,((0,chunk_size-h),(0,chunk_size-w),(0,0)),mode='edge')
            stem = f'HENS00_{x}_{y}'
            Image.fromarray(padded).save(WORK/'source'/(stem+'.png'))
            jobs.append(dict(id=stem,x=x,y=y,width=w,height=h,source_sha256=sha((WORK/'source'/(stem+'.png')).read_bytes())))
    save('plan.json',dict(scene=scene['name'],source_size=list(im.size),scale=4,chunk_size=chunk_size,jobs=jobs,original_tile_roundtrip_exact=True,
         archive_hashes={n:sha((GAME/n).read_bytes()) for n in ('MAP.DAT','MAP.IDX','BLK.DAT','BLK.IDX','COMMON.PAL')}))
    print(json.dumps(dict(directory=str(WORK),jobs=len(jobs),library_tiles=len(tiles),referenced_tiles=len(set(ids.ravel())))))


def finish(diagnostic=False,allow_near4x=False):
    plan = json.loads((WORK/'plan.json').read_text())
    scene,tiles,pal,ids = inputs()
    target = plan['chunk_size']*4
    aligned = False
    for n,h in plan['archive_hashes'].items():
        assert sha((GAME/n).read_bytes()) == h, n
    full = np.zeros((scene['height']*64,scene['width']*64,3),np.uint8)
    for j in plan['jobs']:
        assert sha((WORK/'source'/(j['id']+'.png')).read_bytes()) == j['source_sha256'], 'Source crop changed during generation'
        path = WORK/'generated'/(j['id']+'.png')
        with Image.open(path) as im:
            j['native_size'] = list(im.size)
            j['pixel_alignment'] = 'none'
            if diagnostic:
                im = im.resize((target,target),Image.Resampling.LANCZOS)
                aligned = True
                j['pixel_alignment'] = 'diagnostic-lanczos'
            elif allow_near4x and im.size != (target,target):
                assert im.width == im.height and .97*target <= im.width <= target, f'{path}: expected native size within 3% below {target}; refusing large upscale or downsample'
                # A tiny enlargement duplicates native AI pixels, never filters
                # or averages them. All substantive enlargement must be AI.
                j['pixel_alignment'] = 'nearest-neighbor-enlargement-only'
                j['alignment_scale'] = target/im.width
                ramp = Image.fromarray(np.arange(im.width,dtype=np.int32)[None,:])
                samples = np.asarray(ramp.resize((target,1),Image.Resampling.NEAREST))
                assert np.array_equal(np.unique(samples),np.arange(im.width)), 'Alignment would drop native AI pixels'
                j['all_native_rows_and_columns_retained_during_alignment'] = True
                im = im.resize((target,target),Image.Resampling.NEAREST)
                aligned = True
            else:
                assert im.size == (target,target), f'{path}: native output must be {target}x{target}; refusing resize'
            a = np.asarray(im.convert('RGB'))
        x,y,w,h = (j[k]*4 for k in ('x','y','width','height'))
        full[y:y+h,x:x+w] = a[:h,:w]
        j['generated_sha256'] = sha(path.read_bytes())
    Image.fromarray(full).save(WORK/'context-redraw-4x.png')
    # Only recover tiles whose original visible pixels belong to that tile.
    # Composite pixels cannot reveal an occluded background. Never assign them.
    candidates = {t:[] for t in range(len(tiles))}
    for y in range(scene['height']):
        for x in range(scene['width']):
            bg,fg = ids[:,y,x]
            foreground = np.zeros((16,16),bool) if fg == 1 else tiles[fg] != 255
            bgmask = (tiles[bg] != 255)
            if not np.any(foreground & bgmask):
                candidates[int(bg)].append((x,y,0))
            if fg != 1:
                candidates[int(fg)].append((x,y,1))
    chosen = {}
    records = []
    for t,positions in candidates.items():
        valid = tiles[t] != 255
        mask = np.repeat(np.repeat(valid,4,0),4,1)
        key = 'TILE_'+sha(tiles[t].tobytes())[:24]
        record = dict(tile=t,key=key,occurrences=len(positions),opaque_pixels=int(valid.sum()))
        if positions and valid.any():
            # A medoid minimizes disagreement among different context redraws.
            features = []
            for x,y,l in positions:
                a = full[y*64:(y+1)*64,x*64:(x+1)*64]
                features.append(a.reshape(16,4,16,4,3).mean((1,3))[valid].ravel())
            features = np.asarray(features,np.float32)
            center = np.median(features,axis=0)
            pick = int(np.argmin(np.abs(features-center).mean(axis=1)))
            x,y,layer = positions[pick]
            a = full[y*64:(y+1)*64,x*64:(x+1)*64].copy()
            rgba = np.dstack((a,mask.astype(np.uint8)*255))
            Image.fromarray(rgba).save(WORK/'tiles'/f'{t:04d}.png')
            chosen[t] = rgba
            record.update(status='recovered',source=[x*16,y*16,layer],context_mean_absolute_difference=float(np.abs(features-center).mean()))
        else:
            record['status'] = 'transparent' if not valid.any() else 'unseen-or-occluded'
        records.append(record)
    # A complete, isolated replacement library. Unrecovered tiles use original
    # nearest-neighbor pixels explicitly marked as fallback, never AI coverage.
    atlas = Image.new('RGBA',(16*64,((len(tiles)+15)//16)*64))
    reconstructed = Image.new('RGBA',(scene['width']*64,scene['height']*64))
    library = {}
    pack = WORK/'candidate-pack'
    for directory in ('images','references'):
        (pack/directory).mkdir(parents=True,exist_ok=True)
    manifest = ['# Context-tile candidate pack; offline evaluation']
    palette = bytearray((GAME/'COMMON.PAL').read_bytes())
    raw = (GAME/'MAP.DAT').read_bytes()
    index_raw = (GAME/'MAP.IDX').read_bytes()
    for at in range(0,len(index_raw),16):
        if index_raw[at:at+12].split(b'\0')[0].decode().strip() == scene['palette']:
            p = struct.unpack_from('<I',index_raw,at+12)[0]
            palette[480:765] = raw[p+502:p+787]
            break
    for t in range(len(tiles)):
        a = chosen.get(t)
        if a is None:
            src = np.dstack((pal[tiles[t]],(tiles[t]!=255).astype(np.uint8)*255))
            a = np.repeat(np.repeat(src,4,0),4,1)
        tile = Image.fromarray(a)
        library[t] = tile
        tile.save(WORK/'tiles'/f'{t:04d}.png')
        stem = f'HENS00_TILE_{t:04d}'
        tile.save(pack/'images'/(stem+'.bmp'))
        # Use the original VGA palette bytes, not expanded RGB palette values.
        (pack/'references'/(stem+'.idx')).write_bytes(palette+tiles[t].tobytes())
        manifest.append(f'{stem}\t16\t16\treferences/{stem}.idx\timages/{stem}.bmp\ttruecolor')
        atlas.paste(tile,((t%16)*64,(t//16)*64))
    atlas.save(WORK/'replacement-library-64.png')
    (pack/'manifest.tsv').write_text('\n'.join(manifest)+'\n',encoding='utf-8')
    for layer in (0,1):
        for y in range(scene['height']):
            for x in range(scene['width']):
                t = int(ids[layer,y,x])
                if layer and t == 1: continue
                reconstructed.alpha_composite(library[t],(x*64,y*64))
    reconstructed.save(WORK/'reconstructed-shared-tiles-4x.png')
    for record in records:
        if record['status'] != 'recovered': continue
        t = record['tile']
        x,y,_ = record['source']
        expected = full[y*4:(y+16)*4,x*4:(x+16)*4]
        with Image.open(WORK/'tiles'/f'{t:04d}.png') as im:
            actual = np.asarray(im)
        opaque = actual[:,:,3]!=0
        assert np.array_equal(actual[:,:,:3][opaque],expected[opaque]), f'Tile {t} lost or changed AI pixels'
    reconstructed_rgb = np.asarray(reconstructed.convert('RGB'))
    def boundary_jump(a):
        a = a.astype(np.float32)
        dx = np.abs(a[:,64::64]-a[:,63:-1:64]).mean()
        dy = np.abs(a[64::64]-a[63:-1:64]).mean()
        return float((dx+dy)/2)
    reference = Image.open(GAME/'hd/maps/source/HENS00.png').convert('RGB').resize(reconstructed.size,Image.Resampling.NEAREST)
    preview = Image.new('RGB',(1920,720))
    for i,im in enumerate((reference,Image.fromarray(full),reconstructed.convert('RGB'))):
        im = im.copy()
        im.thumbnail((640,720))
        preview.paste(im,(i*640,0))
    preview.save(WORK/'previews/comparison.png')
    # Full-resolution 320-source-pixel comparison of architectural geometry.
    detail = Image.new('RGB',(3840,1280))
    for i,im in enumerate((reference,Image.fromarray(full),reconstructed.convert('RGB'))):
        detail.paste(im.crop((1280,0,2560,1280)),(i*1280,0))
    detail.save(WORK/'previews/architecture-detail.png')
    report = dict(library_tiles=len(tiles),recovered_tiles=len(chosen),fallback_tiles=[r['tile'] for r in records if r['status']=='unseen-or-occluded'],
                  referenced_tiles=len(set(ids.ravel())),unreferenced_tiles=[i for i in range(len(tiles)) if i not in set(ids.ravel())],
                  records=records,jobs=plan['jobs'],chunk_size=plan['chunk_size'],required_generation_size=[target,target],resampled_generated_images=aligned,
                  diagnostic_only=diagnostic,meets_native_resolution=all(j['native_size']==[target,target] for j in plan['jobs']),
                  meets_requested_approximate_resolution=all(j['native_size'][0]==j['native_size'][1] and .97*target<=j['native_size'][0]<=target for j in plan['jobs']),
                  native_AI_scales=sorted({j['native_size'][0]/plan['chunk_size'] for j in plan['jobs']}),
                  native_AI_pixels_per_original_tile_axis=sorted({j['native_size'][0]/(plan['chunk_size']/16) for j in plan['jobs']}),
                  detail_source='native AI generation',allow_near4x=allow_near4x,
                  tile_candidates_not_averaged=True,selected_tile_opaque_AI_pixels_exact=True,
                  scene_overlap='later edge-aligned window wins; no blending or smoothing',
                  output_sha256={name:sha((WORK/name).read_bytes()) for name in ('context-redraw-4x.png','reconstructed-shared-tiles-4x.png','replacement-library-64.png')},
                  tile_boundary_rgb_jump=dict(direct_context=boundary_jump(full),shared_library=boundary_jump(reconstructed_rgb)),
                  shared_vs_context_rgb_mae=float(np.abs(full.astype(np.float32)-reconstructed_rgb.astype(np.float32)).mean()),
                  original_archives_unchanged=True,original_tile_roundtrip_exact=plan['original_tile_roundtrip_exact'],active_game_modified=False,zero_detail_loss_verified=False,
                  note='Generation is not lossless. Shared medoid tiles discard context variants; occluded/unseen library tiles require more contexts.')
    save('report.json',report)
    print(json.dumps({k:v for k,v in report.items() if k not in ('records','jobs','fallback_tiles')}))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('step',choices=['prepare','finish'])
    parser.add_argument('--diagnostic',action='store_true',help='Explicitly resample for diagnostic comparisons only; never production accepted')
    parser.add_argument('--chunk-size',type=int,choices=[320,640],default=640)
    parser.add_argument('--allow-near4x',action='store_true',help='Accept native AI output up to 3% below exact 4x; enlarge only by nearest neighbor to align tiles')
    args = parser.parse_args()
    if args.chunk_size != 640:
        WORK = GAME/f'hd/maps/context-tiles/HENS00-{args.chunk_size}'
    if args.step == 'prepare': prepare(args.chunk_size)
    else: finish(args.diagnostic,args.allow_near4x)
