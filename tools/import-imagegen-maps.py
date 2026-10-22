"""Register built-in imagegen map atlases and install their extracted tiles.

No image model or API is invoked here. Generated raw images remain intact;
this tool only registers, resizes, slices, verifies and backs up local files.
"""
from pathlib import Path
import argparse
from contextlib import contextmanager
import hashlib
import json
import os
import shutil
import struct

import numpy as np
from PIL import Image, ImageFilter

# Valid original maps can produce up to 16384x16384 owned output pixels.
Image.MAX_IMAGE_PIXELS = 300_000_000

ROOT = Path(__file__).resolve().parents[1]
HD = ROOT/'runtime/game/hd'
WORK = HD/'maps'
ART = WORK/'imagegen'
ACTIVE = HD/'superres'


def digest(data):
    return hashlib.sha256(data).hexdigest()


def read(path):
    return json.loads(path.read_text(encoding='utf-8-sig'))


def save(path, value):
    temporary = path.with_suffix('.tmp')
    temporary.write_text(json.dumps(value, ensure_ascii=False, indent=2), encoding='utf-8')
    temporary.replace(path)


def plan_path(scene=None):
    assert scene is None or 0<len(scene)<=12 and all(c in 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_' for c in scene)
    return ART/'region-jobs.json' if scene in (None, 'HENS00') else ART/'plans'/(scene+'.json')


def plan_maps():
    catalog = read(WORK/'catalog.json')
    (ART/'plans').mkdir(exist_ok=True)
    scenes = []
    for scene in catalog['scenes']:
        name = scene['name']
        if scene['status'] != 'ready':
            scenes.append(dict(scene=name, status=scene['status']))
            continue
        target = plan_path(name)
        w, h = scene['width']*16, scene['height']*16
        if not target.exists():
            source = WORK/'source'/(name+'.png')
            assert source.exists(), source
            jobs = []
            for y in range(0, h, 512):
                for x in range(0, w, 512):
                    box = [max(0, x-32), max(0, y-32), min(w, x+544), min(h, y+544)]
                    job_id = f'{name}_{x}_{y}'
                    jobs.append(dict(id=job_id, box=box, core=[x,y,min(w,x+512),min(h,y+512)],
                                     source=(ART/'region-source'/name/(job_id+'.png')).resolve().as_posix(), status='pending'))
            save(target, dict(scene=name, original_size=[w,h], scale=4, overlap=32,
                              original_source=source.resolve().as_posix(), source_sha256=digest(source.read_bytes()), jobs=jobs))
        jobs = read(target)
        scenes.append(dict(scene=name, plan=target.relative_to(ART).as_posix(), regions=len(jobs['jobs']),
                           source_size=[w,h], status='planned'))
    save(ART/'world-progress.json', dict(method='built-in imagegen', scenes=scenes, full_game_art_complete=False))
    print(f'Planned {sum("plan" in s for s in scenes)} valid scenes.', flush=True)


def _prepare_region(scene, batch):
    path = plan_path(scene)
    plan = read(path)
    job = next(j for j in plan['jobs'] if j['id'] == batch)
    source = Path(job['source'])
    if not source.exists():
        original = Path(plan['original_source'])
        assert digest(original.read_bytes()) == plan['source_sha256']
        with Image.open(original) as image:
            assert list(image.size) == plan['original_size']
            crop = image.crop(job['box']).convert('RGBA')
        source.parent.mkdir(parents=True, exist_ok=True)
        crop.save(source)
        if crop.getbbox() is None:
            job['status'] = 'empty-original'
    job['prepared'] = True
    if job['status'] != 'empty-original':
        source_feature(job)
        cache_path = ART/'source-cache.json'
        cache = read(cache_path) if cache_path.exists() else {}
        reuse = cache.get(job['prepared_source_sha256'])
        if reuse and digest((ART/reuse['generated']).read_bytes()) == reuse['sha256']:
            job.update(status='generated',generated=reuse['generated'],sha256=reuse['sha256'],
                       source_match=reuse['source_match'],source_reused_from=reuse['job'])
    save(path, plan)
    print(json.dumps(job), flush=True)


def prepare_region(scene, batch):
    with registration_lock():
        return _prepare_region(scene,batch)


def image_feature(path):
    with Image.open(path) as image:
        size = image.size
        a = np.asarray(image.convert('RGB').resize((80,80)).filter(ImageFilter.GaussianBlur(1)), np.float32)
    spread = float(a.std((0,1)).max())
    a = (a-a.mean((0,1)))/(a.std((0,1))+1)
    a = a.ravel()
    return a/(np.linalg.norm(a)+1e-8), size, spread


def source_feature(job):
    source = Path(job['source'])
    source_sha = digest(source.read_bytes())
    if job.get('prepared_source_sha256') == source_sha and (ART/job.get('feature','missing')).is_file():
        return np.load(ART/job['feature'],allow_pickle=False),tuple(job['feature_size']),job['feature_spread']
    feature,size,spread = image_feature(source)
    folder = ART/'features'
    folder.mkdir(exist_ok=True)
    path = folder/(job['id']+'.npy')
    np.save(path,feature,allow_pickle=False)
    job.update(prepared_source_sha256=source_sha,feature=path.relative_to(ART).as_posix(),feature_size=list(size),feature_spread=spread)
    return feature,size,spread


def register(batch, source):
    jobs = read(ART/'jobs.json')
    job = next(j for j in jobs['jobs'] if j['id'] == batch)
    with Image.open(source) as im:
        assert im.width == im.height and im.width >= 960, 'Expected square atlas at least 960 pixels wide'
        im.verify()
    (ART/'generated').mkdir(exist_ok=True)
    dest = ART/'generated'/(batch+source.suffix.lower())
    if dest.exists():
        assert digest(dest.read_bytes()) == digest(source.read_bytes()), 'Existing generation differs; retain it before registering a revision'
    else:
        shutil.copy2(source, dest)
    job.update(status='generated', generated=dest.relative_to(ART).as_posix(), sha256=digest(dest.read_bytes()))
    save(ART/'jobs.json', jobs)
    print(f'Registered {batch}: {dest}', flush=True)


@contextmanager
def registration_lock():
    with (ART/'registration.lock').open('a+b') as handle:
        if not handle.tell():
            handle.write(b'\0')
            handle.flush()
        handle.seek(0)
        if os.name == 'nt':
            import msvcrt
            msvcrt.locking(handle.fileno(), msvcrt.LK_LOCK, 1)
        else:
            import fcntl
            fcntl.flock(handle.fileno(), fcntl.LOCK_EX)
        try:
            yield
        finally:
            handle.seek(0)
            if os.name == 'nt':
                msvcrt.locking(handle.fileno(), msvcrt.LK_UNLCK, 1)
            else:
                fcntl.flock(handle.fileno(), fcntl.LOCK_UN)


def register_region(batch, source, scene=None):
    with registration_lock():
        path = plan_path(scene)
        jobs = read(path)
        job = next(j for j in jobs['jobs'] if j['id'] == batch)
        feature, size, _ = image_feature(source)
        matches = []
        for candidate in jobs['jobs']:
            if candidate['status']=='empty-original' or not Path(candidate['source']).exists():
                continue
            ref, dimensions, spread = source_feature(candidate)
            if abs(size[0]/size[1]-dimensions[0]/dimensions[1]) < .12:
                matches.append((float(feature@ref), candidate['id'], spread))
        matches.sort(reverse=True)
        best = matches[0] if matches else None
        if best and best[0]>.65 and (len(matches)==1 or best[0]-matches[1][0]>.10):
            job = next(j for j in jobs['jobs'] if j['id'] == best[1])
            source_match = best[0]
        else:
            expected = next((m for m in matches if m[1] == batch), None)
            assert expected and (expected[0]>.55 or expected[2]<4), 'Generated layout requires review: '+str(matches[:3])
            source_match = expected[0]
        batch = job['id']
        x0, y0, x1, y1 = job['box']
        with Image.open(source) as im:
            assert abs(im.width/im.height-(x1-x0)/(y1-y0)) < 0.12
            im.verify()
        folder = ART/'region-generated'
        folder.mkdir(exist_ok=True)
        dest = folder/(batch+'.png')
        if dest.exists():
            if digest(dest.read_bytes()) != digest(source.read_bytes()):
                review = folder/'review'
                review.mkdir(exist_ok=True)
                alternate = review/(batch+'-'+digest(source.read_bytes())[:12]+'.png')
                shutil.copy2(source,alternate)
                job.setdefault('alternates',[]).append(dict(path=alternate.relative_to(ART).as_posix(),source_match=source_match))
                save(path,jobs)
                print(f'Existing verified region retained; alternate saved for {batch}.', flush=True)
                return
        else:
            shutil.copy2(source, dest)
        job.update(status='generated', generated=dest.relative_to(ART).as_posix(), sha256=digest(dest.read_bytes()),source_match=source_match)
        save(path, jobs)
        cache_path = ART/'source-cache.json'
        cache = read(cache_path) if cache_path.exists() else {}
        cache[job['prepared_source_sha256']] = dict(job=job['id'],generated=job['generated'],sha256=job['sha256'],source_match=source_match)
        save(cache_path,cache)
        print(f'Registered map region {batch}.', flush=True)


def assemble(scene=None):
    path = plan_path(scene)
    jobs = read(path)
    w, h = jobs['original_size']
    scale = jobs['scale']
    assert 0<w<=4096 and 0<h<=4096 and scale==4
    work = ART/'working'
    work.mkdir(exist_ok=True)
    temporary = work/(jobs['scene']+'.rgb')
    canvas = np.memmap(temporary, dtype=np.uint8, mode='w+', shape=(h*scale,w*scale,3))
    regions = []
    for job in jobs['jobs']:
        assert job['status'] in ('generated','empty-original'), job['id']
        x0, y0, x1, y1 = job['box']
        cx0, cy0, cx1, cy1 = job['core']
        if job['status'] != 'empty-original':
            source = ART/job['generated']
            assert digest(source.read_bytes()) == job['sha256']
        xx = np.arange((x1-x0)*scale, dtype=np.float32)/scale+x0
        yy = np.arange((y1-y0)*scale, dtype=np.float32)/scale+y0
        wx, wy = np.ones_like(xx), np.ones_like(yy)
        if cx0>x0: wx *= np.minimum(1, (xx-x0)/(cx0-x0))
        if x1>cx1: wx *= np.minimum(1, (x1-xx)/(x1-cx1))
        if cy0>y0: wy *= np.minimum(1, (yy-y0)/(cy0-y0))
        if y1>cy1: wy *= np.minimum(1, (y1-yy)/(y1-cy1))
        regions.append((job,wx,wy))
    cache = {}
    details = jobs.get('detail_overrides', [])
    for detail in details:
        detail['sha256'] = digest((ART/detail['source']).read_bytes())
    for top in range(0,h*scale,512):
        bottom = min(top+512,h*scale)
        total = np.zeros((bottom-top,w*scale,3),np.float32)
        weights = np.zeros((bottom-top,w*scale),np.float32)
        active = set()
        for job,wx,wy in regions:
            x0,y0,x1,y1 = job['box']
            lo,hi = max(top,y0*scale),min(bottom,y1*scale)
            if lo>=hi: continue
            active.add(job['id'])
            if job['id'] not in cache:
                if job['status']=='empty-original':
                    cache[job['id']] = np.zeros(((y1-y0)*scale,(x1-x0)*scale,3),np.uint8)
                else:
                    with Image.open(ART/job['generated']) as im:
                        cache[job['id']] = np.array(im.convert('RGB').resize(((x1-x0)*scale,(y1-y0)*scale),Image.Resampling.LANCZOS))
            weight = wy[lo-y0*scale:hi-y0*scale,None]*wx[None,:]
            total[lo-top:hi-top,x0*scale:x1*scale] += cache[job['id']][lo-y0*scale:hi-y0*scale]*weight[:,:,None]
            weights[lo-top:hi-top,x0*scale:x1*scale] += weight
        cache = {key:value for key,value in cache.items() if key in active}
        assert np.all(weights>0), 'Incomplete map coverage'
        total /= weights[:,:,None]
        for detail in details:
            x0,y0,x1,y1 = detail['box']
            lo,hi = max(top,y0*scale),min(bottom,y1*scale)
            if lo>=hi: continue
            with Image.open(ART/detail['source']) as im:
                rgb = np.asarray(im.convert('RGB').resize(((x1-x0)*scale,(y1-y0)*scale),Image.Resampling.LANCZOS))[lo-y0*scale:hi-y0*scale]
            yy,xx = np.indices((hi-lo,(x1-x0)*scale),dtype=np.float32)
            yy += lo-y0*scale
            alpha = np.ones(xx.shape,np.float32)
            feather = detail['feather']*scale
            if x0>0: alpha = np.minimum(alpha,xx/feather)
            if y0>0: alpha = np.minimum(alpha,yy/feather)
            if x1<w: alpha = np.minimum(alpha,((x1-x0)*scale-1-xx)/feather)
            if y1<h: alpha = np.minimum(alpha,((y1-y0)*scale-1-yy)/feather)
            alpha = np.clip(alpha,0,1)[:,:,None]
            target = total[lo-top:hi-top,x0*scale:x1*scale]
            target[:] = target*(1-alpha)+rgb*alpha
        canvas[top:bottom] = np.clip(np.rint(total),0,255).astype(np.uint8)
    assembled = ART/'assembled'
    assembled.mkdir(exist_ok=True)
    seed = dict(scene=jobs['scene'], size=jobs['original_size'], scale=scale,
                jobs=[(j['id'],j['box'],j['core'],j.get('sha256'),j['status']) for j in jobs['jobs']],
                details=jobs.get('detail_overrides', []))
    version = digest(json.dumps(seed, sort_keys=True).encode())[:16]
    out = assembled/(jobs['scene']+'-full-'+version+'.png')
    with Image.fromarray(canvas) as image:
        image.save(out)
    canvas.flush()
    canvas._mmap.close()
    del canvas
    temporary.unlink()
    jobs.update(assembled=out.relative_to(ART).as_posix(), assembled_sha256=digest(out.read_bytes()), all_pixels_covered=True)
    save(path, jobs)
    print(f'Assembled {out} ({w*scale}x{h*scale}); all pixels covered.', flush=True)


def bmp(rgb):
    height, width = rgb.shape[:2]
    bgra = np.empty((height, width, 4), np.uint8)
    bgra[:, :, :3] = rgb[:, :, ::-1]
    bgra[:, :, 3] = 255
    payload = bgra.tobytes()
    return b'BM'+struct.pack('<III', 54+len(payload), 0, 54)+struct.pack('<IiiHHIIIIII', 40, width, -height, 1, 32, 0, len(payload), 0, 0, 0, 0)+payload


def scene_chunks(name, source, region=None):
    # Serialize manifest changes with other map imports in this workspace.
    with registration_lock():
        return _scene_chunks(name, source, region)


def _scene_chunks(name, source, region=None):
    """Package a complete scene by position, preserving original layer indices."""
    catalog = read(WORK/'catalog.json')
    scene = next(s for s in catalog['scenes'] if s['name'] == name and s['status'] == 'ready')
    assert name.isalnum() and name.isascii()
    game = HD.parent
    for archive, expected in catalog['source_sha256'].items():
        assert digest((game/archive).read_bytes()) == expected, archive
    raw = (game/'MAP.DAT').read_bytes()
    idx = (game/'MAP.IDX').read_bytes()
    index = {idx[i:i+12].split(b'\0')[0].decode('ascii').strip().upper(): struct.unpack_from('<I', idx, i+12)[0]
             for i in range(0, len(idx), 16)}
    bidx = (game/'BLK.IDX').read_bytes()
    blocks = (game/'BLK.DAT').read_bytes()
    bi = {bidx[i:i+12].split(b'\0')[0].decode('ascii').strip().upper(): struct.unpack_from('<I', bidx, i+12)[0]
          for i in range(0, len(bidx), 16)}
    offset = bi[scene['block']]
    palette = bytearray((game/'COMMON.PAL').read_bytes())
    at = index[scene['palette']]
    palette[480:765] = raw[at+502:at+787]
    w, h = scene['width'], scene['height']
    rx, ry, rw, rh = region if region else (0, 0, w*16, h*16)
    assert all(v % 16 == 0 for v in (rx, ry, rw, rh)) and rw > 0 and rh > 0
    assert rx >= 0 and ry >= 0 and rx+rw <= w*16 and ry+rh <= h*16
    with Image.open(source) as im:
        assert abs(im.width/im.height-rw/rh) < 0.1, 'Generated scene aspect ratio differs'
        if im.size == (rw*4,rh*4) and im.mode=='RGB': image = np.array(im)
        else: image = np.array(im.convert('RGB').resize((rw*4,rh*4),Image.Resampling.LANCZOS))
    folder = ART/'scenes'/name/digest(source.read_bytes())[:16]
    folder.mkdir(parents=True, exist_ok=True)
    rows, records, images = [], [], {}
    pixels = ART/'pixels'
    pixels.mkdir(exist_ok=True)
    for layer in (0, 1):
        indices = np.full((h*16, w*16), 255, np.uint8)
        for y in range(h):
            for x in range(w):
                tile = scene['indices'][layer*w*h+y*w+x]
                if layer and tile == 1:
                    continue
                data = blocks[offset+6+tile*256:offset+6+(tile+1)*256]
                assert len(data) == 256
                data = np.frombuffer(data, np.uint8).reshape(4, 16, 4).transpose(1, 2, 0).reshape(16, 16)
                indices[y*16:(y+1)*16, x*16:(x+1)*16] = data
        for y in range(ry, ry+rh, 192):
            for x in range(rx, rx+rw, 320):
                ref = indices[y:min(y+192, ry+rh), x:min(x+320, rx+rw)]
                hh, ww = ref.shape
                stem = f'{name}_{layer}_{x}_{y}'
                rp = folder/(stem+'.idx')
                rp.write_bytes(palette+ref.tobytes())
                if not layer:
                    payload = bmp(image[(y-ry)*4:(y-ry+hh)*4,(x-rx)*4:(x-rx+ww)*4])
                    ip = pixels/(digest(payload)+'.bmp')
                    if ip.exists(): assert digest(ip.read_bytes())==digest(payload)
                    else: ip.write_bytes(payload)
                    images[(x,y)] = ip
                else: ip = images[(x,y)]
                with Image.open(ip) as im:
                    assert im.size == (ww*4, hh*4)
                rpath, ipath = rp.relative_to(HD).as_posix(), ip.relative_to(HD).as_posix()
                rows.append(f'{name}\t{x}\t{y}\t{ww}\t{hh}\t{rpath}\t{ipath}')
                records.append(dict(layer=layer, x=x, y=y, width=ww, height=hh,
                                    reference=rpath, image=ipath, image_sha256=digest(ip.read_bytes()),
                                    reference_sha256=digest(rp.read_bytes())))
    manifest = ART/'scene-manifest.tsv'
    previous = manifest.read_text(encoding='utf-8').splitlines() if manifest.exists() else []
    new_references = {r.split('\t')[5] for r in rows}
    previous = [r for r in previous if r and not r.startswith('#') and
                (r.split('\t')[5] not in new_references if region else r.split('\t')[0] != name)]
    temporary = manifest.with_suffix('.tmp')
    temporary.write_text('# JX scene regions: map, pixel origin, source dimensions, reference, BMP\n'+'\n'.join(previous+rows)+'\n', encoding='utf-8')
    temporary.replace(manifest)
    report = dict(method='built-in imagegen', scene=name, region=[rx, ry, rw, rh], tiles=rw*rh//256, regions=records,
                  original_layers_and_masks=True, geometry_and_collision_unchanged=True,
                  source=source.resolve().as_posix(), source_sha256=digest(source.read_bytes()), installed=False)
    save(folder/f'verification_{rx}_{ry}_{rw}_{rh}.json', report)
    print(f'Packed coherent scene {name}: {len(rows)} position-specific regions.', flush=True)


def verify_scenes(scene=None):
    catalog = read(WORK/'catalog.json')
    for archive, expected in catalog['source_sha256'].items():
        assert digest((HD.parent/archive).read_bytes()) == expected, archive
    rows = [line.split('\t') for line in (ART/'scene-manifest.tsv').read_text(encoding='utf-8').splitlines()
            if line and not line.startswith('#') and (not scene or line.split('\t')[0] == scene)]
    active_references = {row[5] for row in rows}
    known = {}
    for path in (ART/'scenes').rglob('verification*.json'):
        report = read(path)
        if not any(record['reference'] in active_references for record in report['regions']):
            continue
        assert digest(Path(report['source']).read_bytes()) == report['source_sha256']
        for record in report['regions']:
            known[record['reference']] = record
    checked, scenes = [], set()
    for row in rows:
        name, x, y, w, h, reference, image = row
        x, y, w, h = map(int, (x, y, w, h))
        assert all(v % 16 == 0 for v in (x, y, w, h)) and 0<w<=320 and 0<h<=192
        assert '..' not in reference and '..' not in image
        record = known[reference]
        ref, bmpdata = (HD/reference).read_bytes(), (HD/image).read_bytes()
        assert len(ref) == 768+w*h and digest(ref) == record['reference_sha256']
        assert len(bmpdata) == 54+w*h*64 and digest(bmpdata) == record['image_sha256']
        assert bmpdata[:2] == b'BM' and struct.unpack_from('<HHI', bmpdata, 26) == (1, 32, 0)
        with Image.open(HD/image) as im:
            assert im.size == (w*4, h*4)
            im.verify()
        checked.append(reference)
        scenes.add(name)
    result = dict(method='built-in imagegen', status='verified', scenes=sorted(scenes),
                  regions=len(checked), manifest_sha256=digest((ART/'scene-manifest.tsv').read_bytes()),
                  original_archives_unchanged=True, original_layers_and_masks=True,
                  full_game_art_complete=False)
    if scene:
        (ART/'scene-verification').mkdir(exist_ok=True)
        result['scene_manifest_sha256'] = digest(('\n'.join('\t'.join(row) for row in rows)+'\n').encode())
        save(ART/'scene-verification'/(scene+'.json'), result)
    else:
        save(ART/'verification.json', result)
        (ART/'scene-verification').mkdir(exist_ok=True)
        for name in scenes:
            subset = [row for row in rows if row[0]==name]
            per_scene = dict(result,scenes=[name],regions=len(subset),
                             scene_manifest_sha256=digest(('\n'.join('\t'.join(row) for row in subset)+'\n').encode()))
            save(ART/'scene-verification'/(name+'.json'),per_scene)
    print(json.dumps(result), flush=True)


def progress():
    world = read(ART/'world-progress.json')
    manifest = ART/'scene-manifest.tsv'
    rows = [line.split('\t') for line in manifest.read_text(encoding='utf-8').splitlines() if line and not line.startswith('#')] if manifest.exists() else []
    installation = read(ART/'installation.json') if (ART/'installation.json').exists() else {}
    native = HD.parent/'main_decompiled/build/jx_native.exe'
    support = installation.get('native_map_presentation') and native.exists() and digest(native.read_bytes())==installation.get('native_executable_sha256')
    full_scenes,verified_scenes,generated,total = [],[],0,0
    for item in world['scenes']:
        if 'plan' not in item: continue
        plan = read(ART/item['plan'])
        count = sum(j['status'] in ('generated','empty-original') for j in plan['jobs'])
        generated += count
        total += len(plan['jobs'])
        item.update(completed_regions=count,regions=len(plan['jobs']))
        subset = [row for row in rows if row[0]==item['scene']]
        w,h = plan['original_size']
        coverage = np.zeros((2,h//16,w//16),np.bool_)
        for row in subset:
            _,x,y,rw,rh,reference,_ = row
            layer = int(Path(reference).stem[len(item['scene'])+1:].split('_')[0])
            x,y,rw,rh = map(int,(x,y,rw,rh))
            coverage[layer,y//16:(y+rh)//16,x//16:(x+rw)//16] = True
        complete = bool(np.all(coverage))
        checked = ART/'scene-verification'/(item['scene']+'.json')
        valid = False
        if complete and checked.exists():
            report = read(checked)
            valid = report.get('scene_manifest_sha256')==digest(('\n'.join('\t'.join(row) for row in subset)+'\n').encode())
        if complete: full_scenes.append(item['scene'])
        if valid: verified_scenes.append(item['scene'])
        item['status'] = ('installed' if support else 'verified') if valid else 'packaged' if complete else 'ready-to-assemble' if count==len(plan['jobs']) else 'generating' if any(j.get('prepared') for j in plan['jobs']) else 'planned'
    world.update(completed_regions=generated,total_regions=total,complete_scenes=full_scenes,verified_scenes=verified_scenes,
                 installed_scenes=verified_scenes if support else [],valid_scene_art_complete=len(verified_scenes)==sum('plan' in s for s in world['scenes']))
    save(ART/'world-progress.json',world)
    print(json.dumps({k:world[k] for k in ('completed_regions','total_regions','complete_scenes','verified_scenes','installed_scenes','valid_scene_art_complete')},ensure_ascii=False),flush=True)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('step', choices=('plan-maps', 'prepare-region', 'progress', 'register', 'register-region', 'assemble', 'scene-chunks', 'verify-scenes'))
    parser.add_argument('--batch')
    parser.add_argument('--source', type=Path)
    parser.add_argument('--scene')
    parser.add_argument('--region', help='Original pixel x,y,width,height for a partial map')
    args = parser.parse_args()
    if args.step in ('register', 'register-region'):
        assert args.batch and args.source
        if args.step == 'register': register(args.batch, args.source)
        else: register_region(args.batch, args.source, args.scene)
    elif args.step == 'prepare-region':
        assert args.scene and args.batch
        prepare_region(args.scene, args.batch)
    elif args.step == 'assemble':
        assemble(args.scene)
    elif args.step == 'verify-scenes':
        verify_scenes(args.scene)
    elif args.step == 'scene-chunks':
        assert args.scene and args.source
        region = tuple(map(int, args.region.split(','))) if args.region else None
        assert region is None or len(region) == 4
        scene_chunks(args.scene, args.source, region)
    else:
        globals()[args.step.replace('-', '_')]()
