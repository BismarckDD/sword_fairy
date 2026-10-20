"""Refine the authorized faithful Real-ESRGAN pack using original tile neighbors.

No generated geometry, source archives or executable changes. All work is local.
"""
from pathlib import Path
import argparse, importlib.util, io, json, math, shutil, struct, subprocess, time
import numpy as np
from PIL import Image
spec=importlib.util.spec_from_file_location('remaster_assets',Path(__file__).with_name('remaster-assets.py'))
base=importlib.util.module_from_spec(spec);spec.loader.exec_module(base)
ROOT=base.ROOT; OLD=base.WORK; WORK=ROOT/'runtime/ai-refinement'; GAME=base.GAME
IMAGES=WORK/'images' if (WORK/'images').exists() else GAME/'hd/superres/images'

_references=None
def reference(a):
    global _references
    if _references is None:
        blob=(OLD/'references.bin').read_bytes();_references={}
        for line in (OLD/'manifest.tsv').read_text().splitlines():
            if line.startswith('#'):continue
            key,w,h,offset,_=line.split('\t');at=int(offset[1:]);_references[key]=blob[at:at+768+int(w)*int(h)]
    if a['key'] in _references:return _references[a['key']]
    return (OLD/a['reference']).read_bytes()

def load():
    d=json.loads((OLD/'catalog.json').read_text(encoding='utf-8'))
    return d,{a['key']:a for a in d['assets']}

def prepare():
    d,assets=load()
    for folder in ('input','upscaled','images','previews'): (WORK/folder).mkdir(parents=True,exist_ok=True)
    print("Loading references",flush=True)
    refs={a['key']:reference(a) for a in d['assets'] if a['kind']=='tile'}
    ix={k:np.frombuffer(r[768:],np.uint8).reshape(16,16) for k,r in refs.items()}
    contexts={};origins={}
    def add(grid,kind,source):
        h,w=grid.shape
        for y in range(h):
            for x in range(w):
                key=grid[y,x]
                if key is None or key in contexts or assets[key]['empty']:continue
                patch=np.full((48,48),255,np.uint8)
                for dy in (-1,0,1):
                    for dx in (-1,0,1):
                        yy,xx=y+dy,x+dx
                        if 0<=yy<h and 0<=xx<w and grid[yy,xx] is not None:
                            patch[(dy+1)*16:(dy+2)*16,(dx+1)*16:(dx+2)*16]=ix[grid[yy,xx]]
                assert np.array_equal(patch[16:32,16:32],ix[key])
                contexts[key]=patch;origins[key]=dict(kind=kind,source=source,x=x,y=y)
    print("Assembling walking contexts",flush=True)
    for walk in d['walks']:
        w,h=walk['width_tiles'],walk['height_tiles'];order=walk['order']
        for at in range(0,len(order),w*h):
            grid=np.array([walk['tiles'][i] for i in order[at:at+w*h]],object).reshape(h,w)
            add(grid,'walk',walk['id'])
    # Background and foreground are separate layers, as in the game renderer.
    print("Assembling world contexts",flush=True)
    for world in d['maps']:
        if world['status']!='ready':continue
        w,h=world['width'],world['height'];keys=d['blocks'][world['block']]
        for layer in (0,1):
            values=world['indices'][layer*w*h:(layer+1)*w*h]
            grid=np.array([None if layer and i==1 else keys[i] for i in values],object).reshape(h,w)
            add(grid,'map',world['name'])
    print("Encoding context batches",len(contexts),flush=True)
    records=[];keys=list(contexts)
    for start in range(0,len(keys),100):
        chunk=keys[start:start+100];canvas=np.zeros((math.ceil(len(chunk)/10)*48,480,3),np.uint8)
        filename='context_%04d.png'%(start//100)
        for i,key in enumerate(chunk):
            patch=contexts[key];rgb=base.colors(patch.tobytes(),refs[key][:768],48,48)
            rgb=base.opaque_bleed(rgb,patch!=255)
            x,y=(i%10)*48,(i//10)*48;canvas[y:y+48,x:x+48]=rgb
            records.append(dict(key=key,input=filename,crop=[(x+16)*4,(y+16)*4,(x+32)*4,(y+32)*4],origin=origins[key]))
        Image.fromarray(canvas).save(WORK/'input'/filename)
    base.write_json(WORK/'catalog.json',dict(records=records,source_catalog_sha256=base.digest((OLD/'catalog.json').read_bytes()),
        model='realesr-animevideov3',scale=4,walk_sets=len(d['walks']),map_scenes=sum(m['status']=='ready' for m in d['maps']),
        counts={kind:sum(r['origin']['kind']==kind for r in records) for kind in ('walk','map')}))
    print('Prepared %d contextual tiles in %d batches'%(len(records),math.ceil(len(records)/100)),flush=True)

def prepare_battle():
    d,_=load();catalog=json.loads((WORK/'catalog.json').read_text());catalog['records']=[r for r in catalog['records'] if not r.get('battle')];records=[]
    (WORK/'battle-input').mkdir(exist_ok=True);(WORK/'battle-upscaled').mkdir(exist_ok=True)
    canvas=np.zeros((1024,1024,3),np.uint8);x=y=row_height=batch=0;used=False
    def save():
        Image.fromarray(canvas).save(WORK/'battle-input'/('battle_%04d.png'%batch))
    for a in d['assets']:
        if a['kind']!='battle' or a['empty']:continue
        ref=reference(a);w,h=a['width'],a['height'];indices=np.frombuffer(ref[768:],np.uint8).reshape(h,w)
        yy,xx=np.where(indices!=255);x0,x1=int(xx.min()),int(xx.max())+1;y0,y1=int(yy.min()),int(yy.max())+1
        mask=indices[y0:y1,x0:x1]!=255
        rgb=base.colors(indices.tobytes(),ref[:768],w,h)[y0:y1,x0:x1]
        rgb=np.pad(base.opaque_bleed(rgb,mask),((16,16),(16,16),(0,0)),mode='edge');ph,pw=rgb.shape[:2]
        if x+pw>1024:x=0;y+=row_height;row_height=0
        if y+ph>1024:
            save();batch+=1;canvas[:]=0;x=y=row_height=0
        canvas[y:y+ph,x:x+pw]=rgb;used=True
        records.append(dict(key=a['key'],input='battle_%04d.png'%batch,battle=True,
            crop=[(x+16)*4,(y+16)*4,(x+pw-16)*4,(y+ph-16)*4],bbox=[x0*4,y0*4,x1*4,y1*4],
            origin=dict(kind='battle',source=a['key'])))
        x+=pw;row_height=max(row_height,ph)
    if used:save()
    catalog['records']+=records;catalog['counts']['battle']=len(records)
    base.write_json(WORK/'catalog.json',catalog)
    print('Prepared %d battle frames in %d compact batches'%(len(records),batch+1),flush=True)

def upscale_battle():
    upscale(True)

def upscale(battle=False):
    exe=ROOT/'.cache/ai-upscale/realesrgan/realesrgan-ncnn-vulkan.exe'
    inp=WORK/('battle-input' if battle else 'input');out=WORK/('battle-upscaled' if battle else 'upscaled')
    model='realesrgan-x4plus-anime' if battle else 'realesr-animevideov3'
    cmd=[str(exe),'-i',str(inp),'-o',str(out),'-s','4','-n',model,
        '-m',str(exe.parent/'models'),'-t','128','-g','0','-j','1:1:1','-f','png']
    with (WORK/'inference.log').open('a',encoding='utf-8') as log:
        log.write(json.dumps(cmd)+'\n');log.flush();p=subprocess.Popen(cmd,cwd=exe.parent,stdout=log,stderr=log)
        while p.poll() is None:
            done=len(list(out.glob('*.png')));total=len(list(inp.glob('*.png')))
            base.write_json(WORK/'progress.json',dict(phase='inference',completed=done,total=total))
            print('Context inference %d/%d'%(done,total),flush=True);time.sleep(15)
        if p.returncode:raise RuntimeError('Inference failed: see inference.log')
    for source in inp.glob('*.png'):
        with Image.open(source) as image:size=image.size
        with Image.open(out/source.name) as image:assert image.size==(size[0]*4,size[1]*4);image.verify()
    base.write_json(WORK/'progress.json',dict(phase='battle-inference-complete' if battle else 'context-inference-complete',completed=total,total=total))
    base.write_json(WORK/('battle-model.json' if battle else 'model.json'),dict(name=model,executable_sha256=base.digest(exe.read_bytes()),
        files={p.name:base.digest(p.read_bytes()) for p in (exe.parent/'models').glob(model+'*')}))

def encode_bmp(rgb):
    h,w=rgb.shape[:2];bgra=np.empty((h,w,4),np.uint8);bgra[:,:,:3]=rgb[:,:,::-1];bgra[:,:,3]=255
    payload=bgra.tobytes()
    return b'BM'+struct.pack('<III',54+len(payload),0,54)+struct.pack('<IiiHHIIIIII',40,w,-h,1,32,0,len(payload),0,0,0,0)+payload

def map_blend(rgb,old):
    # Restore old tile edges exactly; a modest interior contribution retains
    # ground texture and avoids a new seam when a shared tile changes neighbors.
    yy,xx=np.indices((64,64));edge=np.minimum.reduce((yy,xx,63-yy,63-xx))
    weight=np.minimum(edge,8).astype(np.uint16)*3
    weight=weight[:,:,None] # 0..24 percent new context
    return ((rgb.astype(np.uint16)*weight+old.astype(np.uint16)*(100-weight)+50)//100).astype(np.uint8)

def soften_maps():
    d=json.loads((WORK/'catalog.json').read_text());done=0;loaded=None;loaded_name=None
    for row in d['records']:
        if row['origin']['kind']!='map' or row.get('map_texture_preserved'):continue
        # Always derive from the saved neural result, so interrupted retries
        # cannot blend an already blended tile a second time.
        if loaded_name!=row['input']:
            if loaded is not None:loaded.close()
            loaded=Image.open(WORK/'upscaled'/row['input']).convert('RGB');loaded_name=row['input']
        path=IMAGES/(row['key']+'.bmp');rgb=np.asarray(loaded.crop(row['crop']))
        old=np.asarray(Image.open(OLD/'images'/path.name).convert('RGB'))
        data=encode_bmp(map_blend(rgb,old));path.write_bytes(data)
        row['output_sha256']=base.digest(data);row['map_texture_preserved']=True;done+=1
        if done%1000==0:
            base.write_json(WORK/'catalog.json',d)
            print('Preserved texture on %d map tiles'%done,flush=True)
    base.write_json(WORK/'catalog.json',d);print('Map texture preservation complete',done,flush=True)

def pack_tiles():
    pack(tiles_only=True)

def pack(tiles_only=False):
    _,assets=load();d=json.loads((WORK/'catalog.json').read_text());loaded_name=None;loaded=None
    processed=0
    for row in d['records']:
        if tiles_only and row.get('battle'):continue
        if row.get('output_sha256'):continue # Full verification reads each completed image once.
        existing=IMAGES/(row['key']+'.bmp')
        if existing.exists():
            data=existing.read_bytes();actual=base.digest(data)
            with Image.open(io.BytesIO(data)) as image:assert image.size==(assets[row['key']]['width']*4,assets[row['key']]['height']*4);image.verify()
            row['output_sha256']=actual;row['reference_sha256']=assets[row['key']]['reference_sha256'];continue
        a=assets[row['key']];r=reference(a)
        assert base.digest(r)==a['reference_sha256']
        filename=('battle-upscaled/' if row.get('battle') else 'upscaled/')+row['input']
        if filename!=loaded_name:
            if loaded:loaded.close()
            loaded=Image.open(WORK/filename).convert('RGB');loaded_name=filename
        rgb=np.asarray(loaded.crop(row['crop'])).copy()
        if row.get('battle'):
            canvas=np.asarray(Image.open(OLD/'images'/(row['key']+'.bmp')).convert('RGB')).copy()
            x0,y0,x1,y1=row['bbox'];previous=canvas[y0:y1,x0:x1].astype(np.uint16)
            assert rgb.shape==previous.shape
            canvas[y0:y1,x0:x1]=((rgb.astype(np.uint16)*65+previous*35+50)//100).astype(np.uint8)
            rgb=canvas
        if row['origin']['kind']=='map':
            old=np.asarray(Image.open(OLD/'images'/(row['key']+'.bmp')).convert('RGB'))
            rgb=map_blend(rgb,old)
        assert rgb.shape==(a['height']*4,a['width']*4,3)
        key=row['key']
        data=encode_bmp(rgb);(IMAGES/(key+'.bmp')).write_bytes(data)
        row['output_sha256']=base.digest(data)
        row['reference_sha256']=a['reference_sha256']
        if row['origin']['kind']=='map':row['map_texture_preserved']=True
        processed+=1
        if processed%1000==0:print('Packed %d new images'%processed,flush=True)
    base.write_json(WORK/'catalog.json',d);print('Pack complete: %d ready images'%sum('output_sha256' in r for r in d['records']),flush=True)

def verify():
    _,assets=load();d=json.loads((WORK/'catalog.json').read_text());changed=0
    assert len({r['key'] for r in d['records']})==len(d['records'])
    for row in d['records']:
        a=assets[row['key']];ref=reference(a);assert base.digest(ref)==row['reference_sha256']
        file=IMAGES/(row['key']+'.bmp');data=file.read_bytes();assert base.digest(data)==row['output_sha256']
        image=np.asarray(Image.open(io.BytesIO(data)).convert('RGB'))
        mask=np.repeat(np.repeat(np.frombuffer(ref[768:],np.uint8).reshape(a['height'],a['width'])!=255,4,0),4,1)
        assert image.shape==(a['height']*4,a['width']*4,3)
        # Runtime masks are still the exact original indexed references, unchanged by this pack.
        assert mask.sum()==int(np.count_nonzero(np.frombuffer(ref[768:],np.uint8)!=255))*16
        changed+=row['output_sha256']!=a['output_sha256']
    edge_samples=0
    for row in d['records']:
        if row['origin']['kind']!='map':continue
        new=np.asarray(Image.open(IMAGES/(row['key']+'.bmp')).convert('RGB'))
        old=np.asarray(Image.open(OLD/'images'/(row['key']+'.bmp')).convert('RGB'))
        assert np.array_equal(new[[0,-1]],old[[0,-1]])
        assert np.array_equal(new[:,[0,-1]],old[:,[0,-1]])
        edge_samples+=1
        if edge_samples==32:break
    original_report=json.loads((OLD/'verification.json').read_text(encoding='utf-8'))
    for name,expected in original_report['source_sha256'].items():assert base.digest((GAME/name).read_bytes())==expected,name
    report=dict(status='verified',catalog_sha256=base.digest((WORK/'catalog.json').read_bytes()),
        manifest_sha256=base.digest((GAME/'hd/superres/manifest.tsv').read_bytes()),
        references_sha256=base.digest((GAME/'hd/superres/references.bin').read_bytes()),map_edge_samples=edge_samples,assets=len(d['records']),changed=changed,counts=d['counts'],original_masks_exact=True,
        walk_sets=d['walk_sets'],map_scenes=d['map_scenes'],source_archives_modified=False,
        source_sha256=original_report['source_sha256'])
    base.write_json(WORK/'verification.json',report);print(json.dumps(report),flush=True)

def review():
    d,assets=load();cache={}
    def tile(key,new):
        ck=(key,new)
        if ck not in cache:
            path=IMAGES/(key+'.bmp')
            if new and path.exists():
                ref=reference(assets[key]);mask=np.repeat(np.repeat(np.frombuffer(ref[768:],np.uint8).reshape(16,16)!=255,4,0),4,1)
                cache[ck]=Image.fromarray(np.dstack((np.asarray(Image.open(path).convert('RGB')),mask.astype(np.uint8)*255)))
            else:
                path=OLD/'previews'/(key+'.png')
                cache[ck]=Image.open(path).convert('RGBA') if path.exists() else Image.new('RGBA',(64,64))
        return cache[ck]
    folder=WORK/'review';folder.mkdir(exist_ok=True)
    for number in (0,1,10,20):
        walk=next(w for w in d['walks'] if w['id']==number)
        w,h=walk['width_tiles'],walk['height_tiles'];nf,af=walk['normal_frames'],walk['alternate_frames'];nd,ad=walk['normal_directions'],walk['alternate_directions']
        width=w*64*max(nf,af);height=h*64*(nd+(ad if af else 0))
        board=Image.new('RGBA',(width*2+16,height),(45,52,56,255))
        for new in (False,True):
            offset=0
            for group,dirs,frames in ((0,nd,nf),(1,ad,af)):
                for direction in range(dirs):
                    for frame in range(frames):
                        for ty in range(h):
                            for tx in range(w):
                                key=walk['tiles'][walk['order'][offset]];offset+=1
                                board.alpha_composite(tile(key,new),((width+16 if new else 0)+(frame*w+tx)*64,((direction+(nd if group else 0))*h+ty)*64))
        board.convert('RGB').save(folder/('walk_%03d_comparison.png'%number))
    map_data=(GAME/'MAP.DAT').read_bytes();map_index=base.read_index('MAP')
    for world in d['maps']:
        if world['name'] not in ('BXD12','HS00','LAN00','XKN00') or world['status']!='ready':continue
        w,h=world['width'],world['height'];keys=d['blocks'][world['block']]
        offset=map_index[world['name']];palette_name=map_data[offset+16:offset+28].split(b'\0')[0].decode('ascii').strip().upper()
        pal=bytearray((GAME/'COMMON.PAL').read_bytes());at=map_index[palette_name];pal[480:765]=map_data[at+502:at+787]
        now=np.frombuffer(pal,np.uint8).reshape(256,3).astype(np.uint32)
        board=Image.new('RGBA',(1280*2+16,768),(45,52,56,255));cx=max(0,(w-20)//2);cy=max(0,(h-12)//2)
        for new in (False,True):
            for layer in (0,1):
                for y in range(min(12,h)):
                    for x in range(min(20,w)):
                        index=world['indices'][layer*w*h+(cy+y)*w+cx+x]
                        if layer and index==1:continue
                        key=keys[index];ref=reference(assets[key]);before=np.frombuffer(ref[:768],np.uint8).reshape(256,3).astype(np.uint32)
                        factors=np.zeros((256,3),np.uint32);bs=before.sum(1);ns=now.sum(1)
                        for i in range(256):
                            for ch in range(3):
                                factors[i,ch]=int(now[i,ch])*256//int(before[i,ch]) if before[i,ch] else int(ns[i])*256//int(bs[i]) if bs[i] else 256 if now[i,ch] else 0
                        rgba=np.array(tile(key,new));indices=np.frombuffer(ref[768:],np.uint8).reshape(16,16)
                        scale=np.repeat(np.repeat(factors[indices],4,0),4,1)
                        rgba[:,:,:3]=np.minimum(rgba[:,:,:3].astype(np.uint32)*scale//256,255).astype(np.uint8)
                        board.alpha_composite(Image.fromarray(rgba),((1296 if new else 0)+x*64,y*64))
        board.convert('RGB').save(folder/(world['name']+'_comparison.jpg'))
    for key in ('BATTLE_FRIEN000_000','BATTLE_ENEMY000_002'):
        a=assets[key];w,h=a['width']*4,a['height']*4;board=Image.new('RGBA',(w*2+16,h),(45,52,56,255))
        for new in (False,True):
            if new:
                ref=reference(a);mask=np.repeat(np.repeat(np.frombuffer(ref[768:],np.uint8).reshape(a['height'],a['width'])!=255,4,0),4,1)
                image=Image.fromarray(np.dstack((np.asarray(Image.open(IMAGES/(key+'.bmp')).convert('RGB')),mask.astype(np.uint8)*255)))
            else:image=Image.open(OLD/'previews'/(key+'.png')).convert('RGBA')
            board.alpha_composite(image,((w+16 if new else 0),0))
        board.convert('RGB').save(folder/(key+'_comparison.jpg'))
    print('Review boards saved to '+str(folder.relative_to(ROOT)),flush=True)

def install():
    report=json.loads((WORK/'verification.json').read_text());assert report['status']=='verified'
    d=json.loads((WORK/'catalog.json').read_text());target=GAME/'hd/superres/images'
    assert base.digest((WORK/'catalog.json').read_bytes())==report['catalog_sha256']
    assert base.digest((target.parent/'manifest.tsv').read_bytes())==report['manifest_sha256']
    assert base.digest((target.parent/'references.bin').read_bytes())==report['references_sha256']
    stamp=time.strftime('%Y%m%d-%H%M%S');backup=ROOT/'runtime/backups'/('before-context-refinement-'+stamp);backup.mkdir(parents=True)
    for name,h in report['source_sha256'].items():assert base.digest((GAME/name).read_bytes())==h
    # Reuse the already verified image directory rather than copying 20,000
    # files a second time. Add unchanged images before the atomic folder swap.
    staged=WORK/'staged'
    assert staged.resolve().parent==WORK.resolve()
    assert IMAGES.resolve()==(WORK/'images').resolve()
    assert target.resolve().parent==(GAME/'hd/superres').resolve()
    assert not target.is_symlink() and not staged.is_symlink() and not IMAGES.is_symlink()
    if staged.exists():raise RuntimeError('Inspect the retained staging directory before retrying')
    expected={r['key']+'.bmp' for r in d['records']}
    for old in target.iterdir():
        assert old.is_file() and not old.is_symlink()
        if old.name not in expected:shutil.copy2(old,IMAGES/old.name)
    IMAGES.rename(staged)
    try:
        target.rename(backup/'images')
        try:staged.rename(target)
        except Exception:
            (backup/'images').rename(target);raise
    except Exception:
        if staged.exists():staged.rename(IMAGES)
        raise
    report.update(installed=True,backup=backup.relative_to(ROOT).as_posix(),manifest_unchanged=True,executable_unchanged=True)
    base.write_json(WORK/'verification.json',report);base.write_json(backup/'deployment.json',report)
    base.write_json(target.parent/'refinement.json',report)
    shutil.copy2(WORK/'catalog.json',target.parent/'refinement-catalog.json')
    print('Installed contextual refinement; backup '+str(backup.relative_to(ROOT)),flush=True)

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('step',choices=('soften_maps','prepare','upscale','prepare_battle','upscale_battle','pack','pack_tiles','verify','review','install'));a=p.parse_args()
    globals()[a.step]()
