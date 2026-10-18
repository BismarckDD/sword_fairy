"""Local, resumable Real-ESRGAN conversion of game-owned indexed assets.

This is neural super-resolution, not image generation or an OpenAI API client.
Original archives, animation timing, indices and transparency are never changed.
"""
from __future__ import annotations
import argparse
import csv
from datetime import datetime, timedelta, timezone
import hashlib
import json
import math
from pathlib import Path
import shutil
import struct
import subprocess
import time

import numpy as np
from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
GAME = ROOT / 'runtime/game'
WORK = ROOT / 'runtime/ai-superres'
U16 = lambda b, p: struct.unpack_from('<H', b, p)[0]
U32 = lambda b, p: struct.unpack_from('<I', b, p)[0]


def digest(data):
    return hashlib.sha256(data).hexdigest()


def write_json(path, value):
    temp = path.with_suffix(path.suffix + '.tmp')
    temp.write_text(json.dumps(value, ensure_ascii=False, indent=2), encoding='utf-8')
    temp.replace(path)


def read_index(name):
    raw = (GAME / (name + '.IDX')).read_bytes()
    assert len(raw) % 16 == 0
    return {raw[i:i+12].split(b'\0')[0].decode('ascii').rstrip().upper(): U32(raw, i+12)
            for i in range(0, len(raw), 16)}


def unplane(raw):
    assert len(raw) == 256
    # Plane-major 16x16 to row-major, matching 1832:13B0.
    return np.frombuffer(raw, dtype=np.uint8).reshape(4, 16, 4).transpose(1, 2, 0).copy().tobytes()


def colors(indices, palette, width, height):
    pal = np.frombuffer(palette, dtype=np.uint8).reshape(256, 3).astype(np.uint16)
    pal = np.minimum(pal * 4, 255).astype(np.uint8)
    pixels = np.frombuffer(indices, dtype=np.uint8).reshape(height, width)
    return pal[pixels]


def opaque_bleed(rgb, mask):
    """Extend edge colors into transparency without changing any visible RGB."""
    out = rgb.copy()
    valid = mask.copy()
    # Deterministic nearest-neighbor propagation; padding is discarded after AI.
    for _ in range(32):
        if valid.all() or not valid.any():
            break
        old = valid.copy()
        for dy, dx in ((-1, 0), (1, 0), (0, -1), (0, 1)):
            shifted = np.roll(old, (dy, dx), axis=(0, 1))
            if dy < 0: shifted[-1, :] = False
            if dy > 0: shifted[0, :] = False
            if dx < 0: shifted[:, -1] = False
            if dx > 0: shifted[:, 0] = False
            fill = shifted & ~valid
            out[fill] = np.roll(out, (dy, dx), axis=(0, 1))[fill]
            valid[fill] = True
    return out


def prepare():
    for folder in ('references', 'input', 'upscaled', 'images', 'previews'):
        (WORK / folder).mkdir(parents=True, exist_ok=True)
    common = (GAME / 'COMMON.PAL').read_bytes()
    assert len(common) == 768
    assets, lookup, uses, maps, walks = [], {}, [], [], []

    def tile(indices, palette, kind, source, number):
        key = digest(indices)
        if key not in lookup:
            name = 'TILE_' + key[:24]
            reference = palette + indices
            (WORK / 'references' / (name + '.idx')).write_bytes(reference)
            lookup[key] = len(assets)
            assets.append(dict(key=name, width=16, height=16, kind='tile', reference='references/'+name+'.idx',
                               reference_sha256=digest(reference), empty=all(i == 255 for i in indices)))
        asset = assets[lookup[key]]['key']
        uses.append(dict(kind=kind, source=source, number=number, asset=asset))
        return asset

    # Walk tiles first so character colors always originate in COMMON.PAL.
    npb = (GAME / 'NPB.DAT').read_bytes()
    npi = (GAME / 'NPB.IDX').read_bytes()
    npc = (GAME / 'NPC.DAT').read_bytes()
    nci = (GAME / 'NPC.IDX').read_bytes()
    inventory = ROOT / 'runtime/ai-redraw/walk-source/inventory.tsv'
    for row in csv.DictReader(inventory.open(encoding='utf-8-sig'), delimiter='\t'):
        if row['status'] != 'exported': continue
        number = int(row['id'])
        b, d = U32(npi, number*4), U32(nci, number*4)
        count = U16(npb, b)
        assert b + 6 + count*256 <= len(npb)
        keys = [tile(unplane(npb[b+6+t*256:b+6+(t+1)*256]), common, 'walk', number, t) for t in range(count)]
        w, h, nf, af, nd, ad = struct.unpack_from('<6H', npc, d)
        refs = [abs(struct.unpack_from('<h', npc, d+18+i*2)[0]) & 4095 for i in range(count)]
        assert all(t < count for t in refs)
        walks.append(dict(id=number, width_tiles=w, height_tiles=h, normal_frames=nf,
                          alternate_frames=af, normal_directions=nd, alternate_directions=ad,
                          tiles=keys, order=refs))

    # Convert every block archive, including valid blocks used by anomalous maps.
    map_data = (GAME / 'MAP.DAT').read_bytes()
    map_index, block_index = read_index('MAP'), read_index('BLK')
    blocks = (GAME / 'BLK.DAT').read_bytes()
    block_keys = {}
    for name, at in block_index.items():
        count = U16(blocks, at)
        assert U16(blocks, at+2) == U16(blocks, at+4) == 16
        assert at+6+count*256 <= len(blocks)
        pal_name = name.rsplit('.', 1)[0] + '.PAL'
        palette = bytearray(common)
        if pal_name in map_index:
            p = map_index[pal_name]
            palette[480:765] = map_data[p+502:p+787]
        assert len(palette) == 768
        block_keys[name] = [tile(unplane(blocks[at+6+t*256:at+6+(t+1)*256]), bytes(palette), 'map', name, t)
                            for t in range(count)]
    for name, at in map_index.items():
        if name.endswith('.PAL'): continue
        w, h = U16(map_data, at), U16(map_data, at+2)
        block = map_data[at+4:at+16].split(b'\0')[0].decode('ascii').strip().upper()
        palette_name = map_data[at+16:at+28].split(b'\0')[0].decode('ascii').strip().upper()
        values = [abs(struct.unpack_from('<h', map_data, at+28+i*2)[0]) & 4095 for i in range(w*h*2)]
        missing = block not in block_keys or palette_name not in map_index or any(t >= len(block_keys.get(block, [])) for t in values)
        maps.append(dict(name=name, width=w, height=h, block=block, palette=palette_name, indices=values,
                         status='source-reference-anomaly' if missing else 'ready'))

    # Battle references have already been sequentially decoded by the LPF exporter.
    battle_root = ROOT / 'runtime/ai-redraw/battle-source'
    battle_count = 0
    for row in csv.DictReader((battle_root/'inventory.tsv').open(encoding='utf-8-sig'), delimiter='\t'):
        assert row['status'] == 'exported', row
        w, h, count = int(row['width']), int(row['height']), int(row['frames'])
        for frame in range(count):
            name = row['animation'] + '_' + format(frame, '03')
            reference = (battle_root/row['animation']/('frame_'+format(frame, '03')+'.idx')).read_bytes()
            assert len(reference) == 768+w*h
            key = 'BATTLE_' + name
            (WORK/'references'/(key+'.idx')).write_bytes(reference)
            assets.append(dict(key=key, width=w, height=h, kind='battle', reference='references/'+key+'.idx',
                               reference_sha256=digest(reference), empty=all(i==255 for i in reference[768:])))
            battle_count += 1

    # Tile inference batches have a 16px guard on every side of each 16px tile.
    # Guard pixels prevent convolution from reading another tile's colors.
    tile_assets = [a for a in assets if a['kind']=='tile' and not a['empty']]
    batches = []
    for start in range(0, len(tile_assets), 100):
        chunk = tile_assets[start:start+100]
        name = 'tiles_' + format(start//100, '04')
        canvas = np.zeros((math.ceil(len(chunk)/10)*48, 480, 3), dtype=np.uint8)
        for i, asset in enumerate(chunk):
            ref = (WORK/asset['reference']).read_bytes()
            rgb = colors(ref[768:], ref[:768], 16, 16)
            mask = np.frombuffer(ref[768:], dtype=np.uint8).reshape(16,16) != 255
            padded = np.pad(opaque_bleed(rgb, mask), ((16,16),(16,16),(0,0)), mode='edge')
            y, x = (i//10)*48, (i%10)*48
            canvas[y:y+48,x:x+48] = padded
            asset['inference'] = name+'.png'
            asset['crop'] = [(x+16)*4,(y+16)*4,(x+32)*4,(y+32)*4]
        Image.fromarray(canvas).save(WORK/'input'/(name+'.png'))
        batches.append(dict(file=name+'.png', width=canvas.shape[1], height=canvas.shape[0], assets=len(chunk)))
    for asset in assets:
        if asset['kind']!='battle' or asset['empty']: continue
        ref = (WORK/asset['reference']).read_bytes()
        w,h = asset['width'],asset['height']
        mask = np.frombuffer(ref[768:],dtype=np.uint8).reshape(h,w) != 255
        rgb = opaque_bleed(colors(ref[768:],ref[:768],w,h), mask)
        padded = np.pad(rgb, ((16,16),(16,16),(0,0)),mode='edge')
        name = asset['key']+'.png'
        Image.fromarray(padded).save(WORK/'input'/name)
        asset['inference'],asset['crop'] = name,[64,64,(w+16)*4,(h+16)*4]
        batches.append(dict(file=name,width=w+32,height=h+32,assets=1))
    total = dict(walks=len(walks), block_archives=len(block_keys), map_tile_uses=sum(len(v) for v in block_keys.values()),
                 walk_tile_uses=sum(len(w['tiles']) for w in walks), unique_tiles=sum(a['kind']=='tile' for a in assets),
                 battle_animations=104,battle_frames=battle_count,inference_images=len(batches),
                 map_reference_anomalies=[m['name'] for m in maps if m['status']!='ready'])
    catalog = dict(schema=1, scale=4, method='Real-ESRGAN neural super-resolution; original geometry and index-255 mask',
                   assets=assets, uses=uses, walks=walks, maps=maps, blocks=block_keys, batches=batches, totals=total)
    write_json(WORK/'catalog.json',catalog)
    write_json(WORK/'progress.json',dict(phase='prepared',totals=total,completed_inference=0))
    print(json.dumps(total,ensure_ascii=False),flush=True)
    extend_objects()


def extend_objects():
    """BOX.IMG contains twelve plane-major world-object tiles, no palette tail."""
    catalog=json.loads((WORK/'catalog.json').read_text(encoding='utf-8'))
    common=(GAME/'COMMON.PAL').read_bytes();raw=(GAME/'BOX.IMG').read_bytes()
    assert raw[:4]==b'IMGV' and len(raw)==16+3072
    lookup={a['key']:a for a in catalog['assets']}
    added=[]
    catalog['uses']=[u for u in catalog['uses'] if u['kind']!='object']
    for t in range(12):
        indices=unplane(raw[16+t*256:16+(t+1)*256]);key='TILE_'+digest(indices)[:24]
        catalog['uses'].append(dict(kind='object',source='BOX.IMG',number=t,asset=key))
        if key in lookup:continue
        reference=common+indices
        (WORK/'references'/(key+'.idx')).write_bytes(reference)
        asset=dict(key=key,width=16,height=16,kind='tile',reference='references/'+key+'.idx',
                   reference_sha256=digest(reference),empty=all(i==255 for i in indices))
        lookup[key]=asset;catalog['assets'].append(asset)
        if not asset['empty']:added.append(asset)
    if added:
        name='object_tiles.png';canvas=np.zeros((48,48*len(added),3),dtype=np.uint8)
        for i,asset in enumerate(added):
            reference=(WORK/asset['reference']).read_bytes();indices=reference[768:]
            rgb=colors(indices,common,16,16);mask=np.frombuffer(indices,dtype=np.uint8).reshape(16,16)!=255
            canvas[:,i*48:(i+1)*48]=np.pad(opaque_bleed(rgb,mask),((16,16),(16,16),(0,0)),mode='edge')
            asset['inference']=name;asset['crop']=[(i*48+16)*4,64,(i*48+32)*4,128]
        Image.fromarray(canvas).save(WORK/'input'/name)
        catalog['batches'].append(dict(file=name,width=canvas.shape[1],height=48,assets=len(added)))
    catalog['totals']['object_tile_uses']=12
    catalog['totals']['unique_tiles']=sum(a['kind']=='tile' for a in catalog['assets'])
    catalog['totals']['inference_images']=len(catalog['batches'])
    write_json(WORK/'catalog.json',catalog)
    print('World object tiles: 12; new AI assets: '+str(len(added)),flush=True)


def upscale(executable, model):
    executable = Path(executable).resolve()
    model_dir = executable.parent/'models'
    model_files = sorted(model_dir.glob(model+'*'))
    assert model_files and executable.is_file()
    catalog = json.loads((WORK/'catalog.json').read_text(encoding='utf-8'))
    state_path = WORK/'inference.json'
    model_info = dict(name=model, executable_sha256=digest(executable.read_bytes()),
                      models={p.name:digest(p.read_bytes()) for p in model_files},
                      source='https://github.com/xinntao/Real-ESRGAN/releases/tag/v0.2.5.0')
    previous_inputs={}
    if state_path.exists():
        prior=json.loads(state_path.read_text(encoding='utf-8'))
        if prior['model']!=model_info: raise RuntimeError('Cached inference uses a different model; use a new work directory')
        previous_inputs={row['file']:row['source_sha256'] for row in prior['inputs']}
    records = []
    pending=WORK/'pending'/str(time.time_ns())
    for batch in catalog['batches']:
        source=WORK/'input'/batch['file'];target=WORK/'upscaled'/batch['file']
        source_hash=digest(source.read_bytes());valid=False
        if target.exists() and previous_inputs.get(batch['file'])==source_hash:
            try:
                with Image.open(target) as im:
                    valid=im.size==(batch['width']*4,batch['height']*4)
                    im.verify()
            except OSError: pass
        if not valid:
            pending.mkdir(parents=True,exist_ok=True)
            shutil.copy2(source,pending/source.name)
        records.append(dict(file=batch['file'],source_sha256=source_hash))
    write_json(state_path,dict(model=model_info,inputs=records))
    if pending.exists() and list(pending.glob('*.png')):
        args=[str(executable),'-i',str(pending),'-o',str(WORK/'upscaled'),'-s','4','-n',model,
              '-m',str(model_dir),'-t','128','-g','0','-j','1:1:1','-f','png']
        with (WORK/'inference.log').open('a',encoding='utf-8') as log:
            log.write('\n'+json.dumps(args)+'\n');log.flush()
            process=subprocess.Popen(args,stdout=log,stderr=log,cwd=str(executable.parent))
            while process.poll() is None:
                done=len(list((WORK/'upscaled').glob('*.png')))
                write_json(WORK/'progress.json',dict(phase='inference',completed_inference=done,total=len(records)))
                print('AI inference %d/%d' % (done,len(records)),flush=True)
                time.sleep(15)
            if process.returncode: raise RuntimeError('Real-ESRGAN failed; see inference.log')
    for batch in catalog['batches']:
        with Image.open(WORK/'upscaled'/batch['file']) as im:
            assert im.size==(batch['width']*4,batch['height']*4),batch['file']
    write_json(WORK/'progress.json',dict(phase='inference-complete',completed_inference=len(records),total=len(records)))
    print('AI inference complete',flush=True)


def bmp(path, rgb):
    h,w,_=rgb.shape
    bgra=np.empty((h,w,4),dtype=np.uint8)
    bgra[:,:,:3]=rgb[:,:,::-1];bgra[:,:,3]=255
    payload=bgra.tobytes()
    header=b'BM'+struct.pack('<III',54+len(payload),0,54)+struct.pack('<IiiHHIIIIII',40,w,-h,1,32,0,len(payload),0,0,0,0)
    path.write_bytes(header+payload)


def pack():
    catalog=json.loads((WORK/'catalog.json').read_text(encoding='utf-8'))
    inference=json.loads((WORK/'inference.json').read_text(encoding='utf-8'))
    lines=['# JXHD1\tReal-ESRGAN 4x; original indexed masks retained']
    loaded_name=None;loaded=None;verified=0;changed=0;empty=0
    for asset in catalog['assets']:
        ref=(WORK/asset['reference']).read_bytes()
        assert digest(ref)==asset['reference_sha256']
        w,h=asset['width'],asset['height']
        if asset['empty']:
            empty+=1
            continue
        if loaded_name!=asset['inference']:
            if loaded is not None: loaded.close()
            loaded=Image.open(WORK/'upscaled'/asset['inference']).convert('RGB');loaded_name=asset['inference']
        rgb=np.asarray(loaded.crop(asset['crop'])).copy()
        assert rgb.shape==(h*4,w*4,3)
        original=colors(ref[768:],ref[:768],w,h)
        baseline=np.repeat(np.repeat(original,4,axis=0),4,axis=1)
        mask=np.repeat(np.repeat(np.frombuffer(ref[768:],dtype=np.uint8).reshape(h,w)!=255,4,axis=0),4,axis=1)
        different=bool(np.any(rgb[mask]!=baseline[mask]));changed+=different
        # Keep exact, hard original geometry. Neural RGB is retained inside it.
        rgba=np.concatenate((rgb,mask[:,:,None].astype(np.uint8)*255),axis=2)
        key=asset['key'];Image.fromarray(rgba).save(WORK/'previews'/(key+'.png'))
        bmp(WORK/'images'/(key+'.bmp'),rgb)
        # Indexed color adaptation is intentional: shared world blocks follow scene palettes.
        lines.append('\t'.join((key,str(w),str(h),asset['reference'],'images/'+key+'.bmp')))
        asset['output_sha256']=digest((WORK/'images'/(key+'.bmp')).read_bytes())
        asset['neural_pixels_differ']=different;verified+=1
    if loaded is not None:loaded.close()
    assert verified+empty==len(catalog['assets'])
    assert verified<65534,'Runtime tag capacity exceeded'
    (WORK/'manifest.tsv').write_text('\n'.join(lines)+'\n',encoding='utf-8')
    report=dict(phase='pack-complete',scale=4,model=inference['model'],totals=catalog['totals'],
                assets_verified=verified,neural_pixels_differ=changed,all_transparent_assets=empty,
                original_mask_preserved=True,source_archives_modified=False,installed=False,
                note='Source-reference anomalies contain unavailable original pixels; all present block archives are converted.')
    write_json(WORK/'catalog.json',catalog);write_json(WORK/'verification.json',report)
    print(json.dumps({k:v for k,v in report.items() if k not in ('model','totals')},ensure_ascii=False),flush=True)


def previews():
    catalog=json.loads((WORK/'catalog.json').read_text(encoding='utf-8'))
    lookup={a['key']:a for a in catalog['assets']}
    cache={}
    def tile_image(key,palette=None):
        cache_key=key if palette is None else (key,digest(palette))
        if cache_key not in cache:
            p=WORK/'previews'/(key+'.png')
            image=Image.open(p).convert('RGBA') if p.exists() else Image.new('RGBA',(64,64))
            if palette is not None and p.exists():
                reference=(WORK/lookup[key]['reference']).read_bytes()
                indices=np.frombuffer(reference[768:],dtype=np.uint8).reshape(16,16)
                before=np.frombuffer(reference[:768],dtype=np.uint8).reshape(256,3).astype(np.uint32)
                now=np.frombuffer(palette,dtype=np.uint8).reshape(256,3).astype(np.uint32)
                base_sum=before.sum(axis=1);now_sum=now.sum(axis=1)
                factors=np.zeros((256,3),dtype=np.uint32)
                for i in range(256):
                    for ch in range(3):
                        factors[i,ch]=int(now[i,ch])*256//int(before[i,ch]) if before[i,ch] else int(now_sum[i])*256//int(base_sum[i]) if base_sum[i] else 256 if now[i,ch] else 0
                rgb=np.array(image);scale=np.repeat(np.repeat(factors[indices],4,axis=0),4,axis=1)
                rgb[:,:,:3]=np.minimum(rgb[:,:,:3].astype(np.uint32)*scale//256,255).astype(np.uint8)
                image=Image.fromarray(rgb)
            cache[cache_key]=image
        return cache[cache_key]
    folder=WORK/'walk-sheets';folder.mkdir(exist_ok=True)
    for walk in catalog['walks']:
        w,h=walk['width_tiles'],walk['height_tiles'];nf,af=walk['normal_frames'],walk['alternate_frames']
        nd,ad=walk['normal_directions'],walk['alternate_directions']
        sheet=Image.new('RGBA',(w*64*max(nf,af),h*64*(nd+(ad if af else 0))))
        offset=0
        for group,dirs,frames in ((0,nd,nf),(1,ad,af)):
            for direction in range(dirs):
                for frame in range(frames):
                    for ty in range(h):
                        for tx in range(w):
                            key=walk['tiles'][walk['order'][offset]];offset+=1
                            sheet.paste(tile_image(key),((frame*w+tx)*64,((direction+(nd if group else 0))*h+ty)*64))
        assert offset==len(walk['order'])
        sheet.save(folder/('walk_'+format(walk['id'],'03')+'.png'))
    # Bounded previews; all other worlds use the same verified block assets at runtime.
    folder=WORK/'world-previews';folder.mkdir(exist_ok=True)
    map_index=read_index('MAP');map_data=(GAME/'MAP.DAT').read_bytes()
    for world in catalog['maps']:
        if world['name'] not in ('BXD12','HS00','LAN00','XKN00') or world['status']!='ready':continue
        w,h=world['width'],world['height'];keys=catalog['blocks'][world['block']]
        offset=map_index[world['name']]
        palette_name=map_data[offset+16:offset+28].split(b'\0')[0].decode('ascii').strip().upper()
        pal=bytearray((GAME/'COMMON.PAL').read_bytes());pal_offset=map_index[palette_name]
        pal[480:765]=map_data[pal_offset+502:pal_offset+787]
        scene=Image.new('RGBA',(w*64,h*64))
        for layer in range(2):
            for y in range(h):
                for x in range(w):
                    index=world['indices'][layer*w*h+y*w+x]
                    if layer and index==1:continue
                    image=tile_image(keys[index],bytes(pal));scene.alpha_composite(image,(x*64,y*64))
        scene.save(folder/(world['name']+'.png'))
    print('Previewed all %d walking sheets and four worlds' % len(catalog['walks']),flush=True)
    # Opaque review images prevent transparent RGB bleed from misleading viewers.
    for number in (164,127,136,298):
        name='walk_'+format(number,'03')+'.png'
        original=Image.open(ROOT/'runtime/ai-redraw/walk-source'/name).convert('RGBA')
        enhanced=Image.open(WORK/'walk-sheets'/name).convert('RGBA')
        original=original.resize(enhanced.size,Image.Resampling.NEAREST)
        w,h=enhanced.size;grid=np.indices((h,w)).sum(axis=0)//8%2
        # Use square checker cells, not diagonal stripes.
        yy,xx=np.indices((h,w));grid=(xx//8+yy//8)%2
        checker=np.where(grid[:,:,None]==0,np.array([40,45,48]),np.array([55,60,64])).astype(np.uint8)
        output=Image.new('RGB',(w*2,h))
        for col,image in enumerate((original,enhanced)):
            back=Image.fromarray(checker).convert('RGBA');back.alpha_composite(image)
            output.paste(back.convert('RGB'),(col*w,0))
        output.save(WORK/'previews'/('review-'+name))


def verify():
    catalog=json.loads((WORK/'catalog.json').read_text(encoding='utf-8'))
    report=json.loads((WORK/'verification.json').read_text(encoding='utf-8'))
    checked=0
    for asset in catalog['assets']:
        reference=(WORK/asset['reference']).read_bytes()
        assert digest(reference)==asset['reference_sha256'],asset['key']
        if asset['empty']:continue
        key=asset['key'];w,h=asset['width'],asset['height']
        data=(WORK/'images'/(key+'.bmp')).read_bytes()
        assert digest(data)==asset['output_sha256'],key
        assert data[:2]==b'BM' and U32(data,10)==54 and U32(data,14)==40,key
        assert struct.unpack_from('<iiHHI',data,18)==(w*4,-h*4,1,32,0),key
        assert len(data)==54+w*h*64,key
        with Image.open(WORK/'previews'/(key+'.png')) as image:
            assert image.size==(w*4,h*4),key
            alpha=np.asarray(image.getchannel('A'))
            mask=np.repeat(np.repeat(np.frombuffer(reference[768:],dtype=np.uint8).reshape(h,w)!=255,4,axis=0),4,axis=1)
            assert np.array_equal(alpha,mask.astype(np.uint8)*255),key
        checked+=1
    assert checked==report['assets_verified']
    if (WORK/'references.bin').exists():
        bundle=(WORK/'references.bin').read_bytes()
        assert bundle[:8]==b'JXREF1\0\0' and U32(bundle,8)==checked
        count=0
        for line in (WORK/'manifest.tsv').read_text(encoding='utf-8').splitlines():
            if not line or line.startswith('#'):continue
            key,w,h,offset,_=line.split('\t')
            assert offset.startswith('@')
            start=int(offset[1:]);length=768+int(w)*int(h)
            assert bundle[start:start+length]==(WORK/'references'/(key+'.idx')).read_bytes()
            count+=1
        assert count==checked
    keys={a['key'] for a in catalog['assets']}
    assert all(u['asset'] in keys for u in catalog['uses'])
    source_names=['NPC.IDX','NPC.DAT','NPB.IDX','NPB.DAT','MAP.IDX','MAP.DAT','BLK.IDX','BLK.DAT','COMMON.PAL','BOX.IMG']
    sources={name:digest((GAME/name).read_bytes()) for name in source_names}
    sources.update({p.relative_to(GAME).as_posix():digest(p.read_bytes()) for p in sorted((GAME/'FIGHT').glob('*.ANM'))})
    report.update(phase='verified',file_hashes_dimensions_and_masks_checked=checked,source_sha256=sources)
    write_json(WORK/'verification.json',report)
    print('Verified %d BMP files, original reference hashes and exact transparency masks' % checked,flush=True)


def bundle_references():
    catalog=json.loads((WORK/'catalog.json').read_text(encoding='utf-8'))
    assets=[a for a in catalog['assets'] if not a['empty']]
    lines=['# JXHD1\tReal-ESRGAN 4x; packed references']
    with (WORK/'references.bin').open('wb') as out:
        out.write(b'JXREF1\0\0'+struct.pack('<I',len(assets)))
        for a in assets:
            offset=out.tell();out.write((WORK/a['reference']).read_bytes())
            lines.append('\t'.join((a['key'],str(a['width']),str(a['height']),'@'+str(offset),'images/'+a['key']+'.bmp')))
    (WORK/'manifest.tsv').write_text('\n'.join(lines)+'\n',encoding='utf-8')
    print('Bundled %d references for a single-file startup load' % len(assets),flush=True)


def install():
    report=json.loads((WORK/'verification.json').read_text(encoding='utf-8'))
    assert report['phase']=='verified','Run verify before install'
    for name,expected in report['source_sha256'].items():
        assert digest((GAME/name).read_bytes())==expected,'Original resource changed: '+name
    build=ROOT/'build/jx_native.exe';active=GAME/'main_decompiled/build/jx_native_hd.exe'
    original=GAME/'main_decompiled/build/jx_native.exe';launcher=ROOT/'build/game_launcher.exe'
    assert build.is_file() and original.is_file() and launcher.is_file()
    hd=GAME/'hd';target=hd/'superres';staging=hd/'superres-staging'
    # Fixed, resolved paths are checked before any directory rename.
    assert target.resolve().parent==hd.resolve() and staging.resolve().parent==hd.resolve()
    assert not target.is_symlink() and not staging.is_symlink()
    if staging.exists():raise RuntimeError('Existing staging directory retained; inspect before retrying install')
    stamp=datetime.now(timezone(timedelta(hours=8))).strftime('%Y%m%d-%H%M%S-%f')
    backup=ROOT/'runtime/backups'/('before-ai-superres-'+stamp);backup.mkdir(parents=True)
    shutil.copy2(original,backup/'jx_native.exe');shutil.copy2(hd/'manifest.tsv',backup/'manifest.tsv')
    if active.exists():shutil.copy2(active,backup/'jx_native_hd.exe')
    old_manifest=(hd/'manifest.tsv').read_bytes()
    custom={}
    for line in old_manifest.decode('utf-8-sig').splitlines():
        row=line.split('\t')
        if len(row)==6 and row[-1]=='truecolor':
            custom[row[4]]=digest((hd/row[4]).read_bytes())
    staging.mkdir()
    for folder in ('images','references'):shutil.copytree(WORK/folder,staging/folder)
    for name in ('manifest.tsv','verification.json','references.bin'):
        if (WORK/name).exists():shutil.copy2(WORK/name,staging/name)
    replaced=False
    try:
        if target.exists():target.rename(backup/'superres');replaced=True
        staging.rename(target)
        temp=active.with_suffix('.exe.new');shutil.copy2(build,temp);temp.replace(active)
        shutil.copy2(launcher,GAME/'启动高清游戏.exe')
    except Exception:
        if target.exists():target.rename(staging)
        if replaced:(backup/'superres').rename(target)
        raise
    assert (hd/'manifest.tsv').read_bytes()==old_manifest
    assert all(digest((hd/name).read_bytes())==expected for name,expected in custom.items())
    report.update(installed=True,backup=backup.relative_to(ROOT).as_posix(),
                  deployed_executable_sha256=digest(active.read_bytes()),preserved_custom_art=len(custom),
                  entry='runtime/game/启动高清游戏.exe',engine='runtime/game/main_decompiled/build/jx_native_hd.exe')
    write_json(WORK/'verification.json',report);write_json(target/'verification.json',report)
    write_json(backup/'deployment.json',report)
    print('Installed %d neural assets; preserved %d existing custom images; backup %s' %
          (report['assets_verified'],len(custom),backup.relative_to(ROOT)),flush=True)


def activate_default():
    report=json.loads((WORK/'verification.json').read_text(encoding='utf-8'))
    assert report['installed']
    backup=ROOT/report['backup'];original=GAME/'main_decompiled/build/jx_native.exe'
    source=ROOT/'build/jx_native.exe'
    assert backup.resolve().is_relative_to((ROOT/'runtime/backups').resolve())
    assert digest(original.read_bytes()) in (digest((backup/'jx_native.exe').read_bytes()),digest(source.read_bytes()))
    launcher=GAME/'启动游戏.exe'
    if not (backup/'启动游戏.exe').exists():shutil.copy2(launcher,backup/'启动游戏.exe')
    for src,dest in ((source,original),(ROOT/'build/game_launcher.exe',launcher)):
        temp=dest.with_suffix('.exe.new');shutil.copy2(src,temp);temp.replace(dest)
    report.update(entry='runtime/game/启动游戏.exe',alternate_entry='runtime/game/启动高清游戏.exe',
                  default_engine_sha256=digest(original.read_bytes()))
    write_json(WORK/'verification.json',report);write_json(GAME/'hd/superres/verification.json',report)
    write_json(backup/'deployment.json',report)
    print('Updated the normal launcher and engine; original executables retained in '+report['backup'],flush=True)


def refresh_runtime():
    report=json.loads((WORK/'verification.json').read_text(encoding='utf-8'))
    assert report['installed'] and report['phase']=='verified'
    for name,expected in report['source_sha256'].items():assert digest((GAME/name).read_bytes())==expected,name
    for name in ('jx_native.exe','jx_native_hd.exe'):
        destination=GAME/'main_decompiled/build'/name;temp=destination.with_suffix('.exe.new')
        shutil.copy2(ROOT/'build/jx_native.exe',temp);temp.replace(destination)
    addon=GAME/'hd/superres'
    for name in ('references.bin','manifest.tsv'):
        temp=addon/(name+'.new');shutil.copy2(WORK/name,temp);temp.replace(addon/name)
    report.update(packed_reference_sha256=digest((addon/'references.bin').read_bytes()),
                  deployed_executable_sha256=digest((GAME/'main_decompiled/build/jx_native_hd.exe').read_bytes()),
                  default_engine_sha256=digest((GAME/'main_decompiled/build/jx_native.exe').read_bytes()))
    write_json(WORK/'verification.json',report);write_json(addon/'verification.json',report)
    print('Refreshed runtime binaries and packed reference catalog',flush=True)


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('phase',choices=('prepare','extend-objects','upscale','pack','previews','bundle','verify','install','activate-default','refresh-runtime'))
    parser.add_argument('--executable',default=str(ROOT/'.cache/ai-upscale/realesrgan/realesrgan-ncnn-vulkan.exe'))
    parser.add_argument('--model',default='realesr-animevideov3')
    args=parser.parse_args()
    {'prepare':prepare,'extend-objects':extend_objects,'upscale':lambda:upscale(args.executable,args.model),
     'pack':pack,'previews':previews,'bundle':bundle_references,'verify':verify,'install':install,
     'activate-default':activate_default,'refresh-runtime':refresh_runtime}[args.phase]()

if __name__=='__main__':main()
