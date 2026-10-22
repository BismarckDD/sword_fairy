"""Register supplied/generated character sheets with consistent height.
No artwork is generated here. Built-in imagegen creates the source PNGs.
"""
from pathlib import Path
import argparse,csv,hashlib,json,struct,time
import numpy as np
from PIL import Image,ImageDraw
ROOT=Path(__file__).resolve().parents[1];GAME=ROOT/'runtime/game';HD=GAME/'hd';ART=HD/'sprites';PACK=HD/'walk-redraw'

def digest(data):return hashlib.sha256(data).hexdigest()

def inputs():
 """Enlarge reference pixels without changing their colors or pose."""
 destination=ART/'inputs';destination.mkdir(exist_ok=True)
 for a in plan()['items']:
  if a['kind']=='non-character':continue
  image=Image.open(a['source']).convert('RGBA')
  if a['retiled']:
   arranged=Image.new('RGBA',(a['gen_columns']*a['width'],a['gen_rows']*a['height']))
   for k,pose in enumerate(poses(a)):
    cell=image.crop((pose['x']*a['width'],pose['y']*a['height'],(pose['x']+1)*a['width'],(pose['y']+1)*a['height']))
    arranged.paste(cell,((k%a['gen_columns'])*a['width'],(k//a['gen_columns'])*a['height']))
   image=arranged
  pixels=np.array(image);pixels[pixels[:,:,3]==0]=0;image=Image.fromarray(pixels)
  factor=max(1,min(16,1536//max(image.size)))
  image.resize((image.width*factor,image.height*factor),Image.Resampling.NEAREST).save(destination/('walk_%03d.png'%a['id']))
 print('Prepared enlarged original references',flush=True)
def write_json(path,data):
 temp=path.with_suffix(path.suffix+'.new');temp.write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n',encoding='utf-8');temp.replace(path)
def plan():return json.loads((ART/'plan.json').read_text(encoding='utf-8'))
def poses(a):
 result=[]
 for group,(dirs,frames) in enumerate(((a['normal_directions'],a['normal_frames']),(a['alternate_directions'],a['alternate_frames']))):
  for direction in range(dirs):
   for frame in range(frames):result.append(dict(group=group,direction=direction,frame=frame,x=frame,y=direction+(a['normal_directions'] if group else 0)))
 return result

def row_cuts(image,rows):
 if rows==1:return [0,image.height]
 # Faint antialiasing between rows is not solid artwork. Use the opaque
 # body boundary to find gutters while retaining the original RGBA pixels.
 alpha=np.asarray(image)[:,:,3];counts=(alpha>240).sum(1);gaps=[];start=None
 for y,count in enumerate(counts):
  if count<=max(3,image.width//400) and start is None:start=y
  if count>max(3,image.width//400) and start is not None:
   if y-start>3:gaps.append((start,y))
   start=None
 cuts=[0]
 for row in range(1,rows):
  expected=image.height*row/rows;candidates=[g for g in gaps if abs((g[0]+g[1])/2-expected)<image.height/rows*.46]
  if not candidates:raise ValueError('No transparent row gutter at row %d'%row)
  cuts.append(sum(min(candidates,key=lambda g:abs((g[0]+g[1])/2-expected)))//2)
 cuts.append(image.height)
 if cuts!=sorted(set(cuts)):raise ValueError('Missing/repeated sprite row')
 return cuts

def import_one(a):
 id=a['id'];source=ART/'sources'/('walk_%03d.png'%id);record=ART/'records'/('walk_%03d.json'%id)
 layout_file=ART/'layouts'/('walk_%03d.json'%id)
 if not source.exists() or not layout_file.exists():return 'pending'
 source_stat=source.stat();layout_stat=layout_file.stat()
 plan_hash=digest(json.dumps(a,sort_keys=True).encode('utf-8'))
 layout=json.loads(layout_file.read_text(encoding='utf-8-sig'))
 overlay_file=ART/layout['registered_overlay'] if layout.get('registered_overlay') else None
 stamp=[source_stat.st_size,source_stat.st_mtime_ns,layout_stat.st_size,layout_stat.st_mtime_ns]
 if overlay_file:
  overlay_stat=overlay_file.stat();stamp.extend([overlay_stat.st_size,overlay_stat.st_mtime_ns])
 if record.exists():
  prior=json.loads(record.read_text(encoding='utf-8'))
  if prior.get('status')=='ready' and prior.get('source_stamp')==stamp and prior.get('registration_version')==8 and prior.get('plan_sha256')==plan_hash and (HD/prior['binary']).is_file() and (prior.get('art_width')!=96 or Path(str(HD/prior['binary'])+'.wide').is_file()):return 'ready'
 source_hash=digest(source.read_bytes());layout_hash=digest(layout_file.read_bytes()+(overlay_file.read_bytes() if overlay_file else b''))
 if record.exists():
  prior=json.loads(record.read_text(encoding='utf-8'))
  if prior.get('status')=='ready' and prior.get('source_sha256')==source_hash and prior.get('registration_version')==8 and prior.get('plan_sha256')==plan_hash and prior.get('layout_sha256')==layout_hash and (HD/prior['binary']).is_file() and (prior.get('art_width')!=96 or Path(str(HD/prior['binary'])+'.wide').is_file()):return 'ready'
 image=Image.open(source).convert('RGBA');rgba=np.asarray(image);transparent=float((rgba[:,:,3]<32).mean())
 if transparent<.08:raise ValueError('Source has insufficient real transparency')
 layout=json.loads(layout_file.read_text(encoding='utf-8-sig')) if layout_file.exists() else {}
 columns,rows=layout.get('columns',a['gen_columns']),layout.get('rows',a['gen_rows']);cuts=row_cuts(image,rows)
 sequence=poses(a);bodies=[];boxes=[];original=Image.open(a['source']).convert('RGBA')
 fw,fh=a['width']*4,a['height']*4;foot=[];original_heights=[]
 art_width=layout.get('wide_width',fw)
 overlay=Image.open(overlay_file).convert('RGBA') if overlay_file else None
 if overlay is not None and overlay.size!=(art_width*a['columns'],fh*a['rows']):raise ValueError('Registered overlay size mismatch')
 if art_width!=fw and (fw!=64 or art_width!=96):raise ValueError('Wide artwork requires a single-column 96px drawing cell')
 for k,pose in enumerate(sequence):
  if layout.get('pose_map'):x,y=layout['pose_map'][k]
  else:x,y=(k%columns,k//columns) if layout.get('retiled',a['retiled']) else (pose['x'],pose['y'])
  cell=image.crop((x*image.width//columns,cuts[y],(x+1)*image.width//columns,cuts[y+1]))
  old=np.asarray(original.crop((pose['x']*a['width'],pose['y']*a['height'],(pose['x']+1)*a['width'],(pose['y']+1)*a['height'])))[:,:,3]
  yy,xx=np.where(old>128)
  if not len(xx):bodies.append(None);boxes.append(None);foot.append(fh);original_heights.append(0);continue
  ys,xs=np.where(np.asarray(cell)[:,:,3]>200)
  if not len(xs):raise ValueError('Missing source pose %d'%k)
  box=(int(xs.min()),int(ys.min()),int(xs.max())+1,int(ys.max())+1)
  bodies.append(cell.crop(box));boxes.append(box);foot.append((int(yy.max())+1)*4);original_heights.append(int(yy.max()-yy.min()+1))
 valid=[b for b in bodies if b is not None]
 if not valid:raise ValueError('No visible poses')
 max_w,max_h=max(b.width for b in valid),max(b.height for b in valid)
 # Upright adult sheets use one common physical height. Posed, fragment and
 # animal sheets retain the original relative extent, never inflated upright.
 # Preserve the game's four-times scale for special poses. Full human poses
 # in ordinary cells share the adult ceiling instead of being shrunk again.
 target=160 if a['standing'] else min(fh-4,max(original_heights)*4)
 if a['kind']=='character' and a['height']<=48:target=min(target,160)
 # A few single-person action resources have an unusually large native cell.
 # Their reviewed target keeps the person comparable to ordinary map actors.
 if a.get('target_height'):target=a['target_height']
 target=min(target,fh-4)
 # One scale for both axes and every frame. Explicit wide layouts retain
 # the native core plus a presentation-only margin instead of squeezing it.
 height_scale=target/max_h
 scale=min(height_scale,(art_width-4)/max_w)
 horizontal=1.0
 normal=Image.new('RGBA',(art_width*a['columns'],fh*a['rows']));tiles=[];heights=[];wide_tiles=None
 npc=(GAME/'NPC.DAT').read_bytes();ix=(GAME/'NPC.IDX').read_bytes();at=struct.unpack_from('<I',ix,id*4)[0]
 w,h,nf,af,nd,ad=struct.unpack_from('<6H',npc,at);n=w*h*len(sequence);order=[abs(v)&4095 for v in struct.unpack_from('<%dh'%n,npc,at+18)]
 if sorted(order)!=list(range(n)):raise ValueError('Unsupported non-permutation NPC tile table')
 tiles=[None]*n
 if art_width!=fw:wide_tiles=[None]*n
 for k,(pose,body) in enumerate(zip(sequence,bodies)):
  cell=Image.new('RGBA',(art_width,fh))
  if body is not None:
   body=body.resize((max(1,round(body.width*scale*horizontal)),max(1,round(body.height*scale))),Image.Resampling.LANCZOS)
   if body.width>art_width or body.height>fh:raise ValueError('Registered pose outside frame')
   # Uniform character scale; only the source frame's original foot level varies.
   bottom=max(body.height,min(fh,foot[k]));cell.alpha_composite(body,((art_width-body.width)//2,bottom-body.height));heights.append(body.height)
  for patch in layout.get('registered_overlay_regions',[]):
   if patch['frame']==k:
    if overlay is None:raise ValueError('Missing registered overlay')
    x0,y0,x1,y1=patch['box'];assert 0<=x0<x1<=art_width and 0<=y0<y1<=fh
    ox,oy=pose['x']*art_width,pose['y']*fh
    cell.paste(overlay.crop((ox+x0,oy+y0,ox+x1,oy+y1)),(x0,y0))
  normal.alpha_composite(cell,(pose['x']*art_width,pose['y']*fh))
  for ty in range(h):
   for tx in range(w):
    index=order[k*w*h+ty*w+tx]
    inset=(art_width-fw)//2
    tiles[index]=np.asarray(cell.crop((inset+tx*64,ty*64,inset+(tx+1)*64,(ty+1)*64)))[:,:,[2,1,0,3]].tobytes()
    if wide_tiles is not None:wide_tiles[index]=np.asarray(cell.crop((0,ty*64,art_width,(ty+1)*64)))[:,:,[2,1,0,3]].tobytes()
 data=b'JXWALK1\0'+struct.pack('<I',n)+(GAME/'COMMON.PAL').read_bytes()+b''.join(tiles)
 binary=PACK/('walk_%03d-painted.bin'%id);temp=binary.with_suffix('.bin.new');temp.write_bytes(data);temp.replace(binary)
 wide_path=Path(str(binary)+'.wide')
 if wide_tiles is not None:
  wide_data=b'JXWIDE1\0'+struct.pack('<II',n,art_width)+b''.join(wide_tiles)
  temp=Path(str(wide_path)+'.new');temp.write_bytes(wide_data);temp.replace(wide_path)
 elif wide_path.exists():wide_path.unlink()
 sheet=ART/'sheets'/('walk_%03d.png'%id);normal.save(sheet)
 preview=Image.new('RGBA',normal.size,(45,52,56,255));preview.alpha_composite(normal);preview.convert('RGB').save(ART/'previews'/('walk_%03d.png'%id))
 report=dict(id=id,status='ready',registration_version=8,source_sha256=source_hash,layout_sha256=layout_hash,source_stamp=stamp,binary=binary.relative_to(HD).as_posix(),binary_sha256=digest(data),tile_count=n,frames=len(sequence),width=fw,height=fh,standing=a['standing'],target_height=target,height_min=min(heights),height_max=max(heights),height_scale=scale,horizontal_fit=horizontal,uniform_fit=scale/height_scale,art_width=art_width,rows=rows,columns=columns,alpha='supplied/generated RGBA, independent of old indexed mask',original_archives_modified=False)
 report['plan_sha256']=plan_hash
 if a.get('safe_variant'):report['variant']=a['safe_variant']
 write_json(record,report);return 'ready'

def geometry_pack(d,records):
 inventory={a['id']:a for a in d['items']};npc=(GAME/'NPC.DAT').read_bytes();ix=(GAME/'NPC.IDX').read_bytes();data=bytearray(b'JXWGEO1\0'+struct.pack('<I',len(records)))
 for id in sorted(records):
  a=inventory[id];n=records[id]['tile_count'];area=(a['width']//16)*(a['height']//16);at=struct.unpack_from('<I',ix,id*4)[0]
  order=[abs(v)&4095 for v in struct.unpack_from('<%dh'%n,npc,at+18)];assert sorted(order)==list(range(n))
  positions=[0]*n
  for k,raw in enumerate(order):positions[raw]=k%area
  flags=(1 if a.get('foreground_reference') else 0)|(2 if id==94 else 0);extra=31 if id==94 else 0
  data+=struct.pack('<3H2B',id,n,area,flags,extra)+struct.pack('<%dH'%n,*positions)
 temp=PACK/'geometry.bin.new';temp.write_bytes(data);temp.replace(PACK/'geometry.bin')

def build_manifest(d):
 records={}
 for f in (ART/'records').glob('walk_*.json'):
  r=json.loads(f.read_text(encoding='utf-8'))
  if r.get('status')=='ready':records[r['id']]=r
 lines=['# JXWALK-MANIFEST1\tresource\ttiles\tfile'];covered=[]
 for id in sorted(set(records)|{int(k) for k in d['aliases']}):
  canonical=int(d['aliases'].get(str(id),id));r=records.get(canonical)
  if r:
   lines.append('%d\t%d\t%s'%(id,r['tile_count'],Path(r['binary']).name));covered.append(id)
 temp=PACK/'manifest.tsv.new';temp.write_text('\n'.join(lines)+'\n',encoding='utf-8');temp.replace(PACK/'manifest.tsv')
 geometry_pack(d,records)
 pending=[a['id'] for a in d['items'] if a['kind']!='non-character' and a['id'] not in records]
 report=dict(unique_character_sources=d['character_sources'],registered=len(records),resource_ids_covered=len(covered),pending=pending,complete=not pending,adult_target_height=160,adult_height_tolerance=12,all_generated_art_inside_requested_hd_directory=True)
 write_json(ART/'progress.json',report);print(json.dumps({k:v for k,v in report.items() if k!='pending'}|dict(pending_count=len(pending))),flush=True);return report

def run(selected=None):
 d=plan()
 for name in ('records','sheets','previews'):(ART/name).mkdir(exist_ok=True)
 PACK.mkdir(exist_ok=True)
 errors=[]
 for a in d['items']:
  if a['kind']=='non-character' or selected is not None and a['id']!=selected:continue
  try:status=import_one(a)
  except Exception as e:
   write_json(ART/'records'/('walk_%03d.json'%a['id']),dict(id=a['id'],status='rejected',error=str(e)))
   errors.append(dict(id=a['id'],error=str(e)));print('Registration issue',a['id'],str(e),flush=True)
 write_json(ART/'registration-issues.json',errors);return build_manifest(d)

def verify():
 d=plan();records=[];palette=(GAME/'COMMON.PAL').read_bytes()
 for f in (ART/'records').glob('walk_*.json'):
  r=json.loads(f.read_text(encoding='utf-8'))
  if r.get('status')!='ready':continue
  data=(HD/r['binary']).read_bytes()
  assert digest(data)==r['binary_sha256'] and data[:8]==b'JXWALK1\0'
  assert struct.unpack_from('<I',data,8)[0]==r['tile_count'] and len(data)==780+r['tile_count']*16384 and data[12:780]==palette
  if r.get('art_width')==96:
   wide=Path(str(HD/r['binary'])+'.wide').read_bytes();assert wide[:8]==b'JXWIDE1\0'
   assert struct.unpack_from('<II',wide,8)==(r['tile_count'],96) and len(wide)==16+r['tile_count']*96*64*4
   for tile in range(r['tile_count']):
    full=np.frombuffer(wide,dtype=np.uint8,offset=16+tile*96*64*4,count=96*64*4).reshape(64,96,4)
    core=np.frombuffer(data,dtype=np.uint8,offset=780+tile*64*64*4,count=64*64*4).reshape(64,64,4)
    assert np.array_equal(full[:,16:80],core)
  if r['standing']:assert 148<=r['height_max']<=172 and r['horizontal_fit']>=.65
  records.append(r)
 geometry=(PACK/'geometry.bin').read_bytes();assert geometry[:8]==b'JXWGEO1\0';assert struct.unpack_from('<I',geometry,8)[0]==len(records)
 by_id={r['id']:r for r in records};inventory={a['id']:a for a in d['items']};npc=(GAME/'NPC.DAT').read_bytes();ix=(GAME/'NPC.IDX').read_bytes();seen=set();offset=12
 for _ in records:
  id,n,area,flags,extra=struct.unpack_from('<3H2B',geometry,offset);offset+=8;assert id in by_id and id not in seen and n==by_id[id]['tile_count']
  a=inventory[id];assert flags==((1 if a.get('foreground_reference') else 0)|(2 if id==94 else 0)) and extra==(31 if id==94 else 0)
  assert area==(a['width']//16)*(a['height']//16);positions=struct.unpack_from('<%dH'%n,geometry,offset);offset+=n*2;at=struct.unpack_from('<I',ix,id*4)[0]
  order=[abs(v)&4095 for v in struct.unpack_from('<%dh'%n,npc,at+18)];assert all(positions[raw]==k%area for k,raw in enumerate(order));seen.add(id)
 assert offset==len(geometry) and len(seen)==len(records)
 report=dict(status='PASS' if len(records)==d['character_sources'] else 'PARTIAL',registered=len(records),expected=d['character_sources'],complete=len(records)==d['character_sources'],adult_height_max_range=[min((r['height_max'] for r in records if r['standing']),default=0),max((r['height_max'] for r in records if r['standing']),default=0)],all_bin_sizes_and_hashes_checked=True,geometry_records_checked=len(seen),geometry_sha256=digest(geometry))
 write_json(ART/'verification.json',report);print(json.dumps(report),flush=True)

def contacts():
 ready={r['id'] for f in (ART/'records').glob('walk_*.json') if (r:=json.loads(f.read_text(encoding='utf-8'))).get('status')=='ready'}
 files=sorted(f for f in (ART/'previews').glob('walk_*.png') if int(f.stem[5:]) in ready)
 for start in range(0,len(files),24):
  board=Image.new('RGB',(960,1200),(45,52,56));draw=ImageDraw.Draw(board)
  for j,f in enumerate(files[start:start+24]):
   x=j%6*160;y=j//6*300;image=Image.open(f).convert('RGB');image.thumbnail((150,270),Image.Resampling.LANCZOS);board.paste(image,(x+(160-image.width)//2,y+25));draw.text((x+5,y+5),f.stem,fill='white')
  board.save(ART/'contacts'/('registered_%02d.jpg'%(start//24)))
 records=sorted((r for f in (ART/'records').glob('walk_*.json') if (r:=json.loads(f.read_text(encoding='utf-8'))).get('status')=='ready' and r['standing']),key=lambda r:r['id'])
 inventory={a['id']:a for a in plan()['items']}
 for start in range(0,len(records),32):
  board=Image.new('RGB',(1152,880),(45,52,56));draw=ImageDraw.Draw(board)
  for j,r in enumerate(records[start:start+32]):
   x=j%8*144;y=j//8*220;a=inventory[r['id']];image=Image.open(ART/'sheets'/('walk_%03d.png'%r['id'])).convert('RGBA')
   top=r['height'] if a['normal_frames'] and a['normal_directions']==4 else 0
   cell=image.crop((0,top,r['width'],top+r['height']))
   box=cell.getbbox()
   if box:cell=cell.crop(box)
   draw.line((x,y+195,x+143,y+195),fill=(90,100,105));draw.line((x,y+35,x+143,y+35),fill=(70,80,85))
   board.paste(cell,(x+(144-cell.width)//2,y+195-cell.height),cell)
   draw.text((x+5,y+201),'%03d  h=%d'%(r['id'],r['height_max']),fill='white')
  board.save(ART/'contacts'/('heights_%02d.jpg'%(start//32)))
 print('Registered contact pages',(len(files)+23)//24,flush=True)

def watch():
 while True:
  report=run()
  if report['complete']:
   verify();contacts();catalog();return
  if (ART/'stop-importer').exists():return
  time.sleep(15)

def catalog():
 d=plan();items={a['id']:a for a in d['items']};records={}
 for f in (ART/'records').glob('walk_*.json'):
  r=json.loads(f.read_text(encoding='utf-8'));records[r['id']]=r
 names={};source=ROOT/'runtime/ai-redraw/character-sprites.csv';translated={'buttfly':'蝴蝶','fish':'鱼','chicken':'鸡','dog':'狗','snowman':'雪人'}
 if source.exists():
  for row in csv.DictReader(source.open(encoding='utf-8-sig')):
   name=row['name'].strip()
   try:id=int(row['sprite'])
   except (ValueError,TypeError):continue
   if name:names.setdefault(id,set()).add(translated.get(name,name))
 aliases={int(k):int(v) for k,v in d['aliases'].items()};ids={id for id,a in items.items() if a['kind']!='non-character'}
 ids|={id for id,base in aliases.items() if base in ids}
 with (ART/'角色索引.csv').open('w',encoding='utf-8-sig',newline='') as f:
  writer=csv.writer(f);writer.writerow(['编号','角色或名称','类型','透明动作表','高清原图','状态','最高帧高度','备注'])
  for id in sorted(ids):
   base=aliases.get(id,id);a=items[base];r=records.get(base,{});status='已完成' if r.get('status')=='ready' else '需修正' if r.get('status')=='rejected' else '待处理'
   kind={'character':'人物','animal':'动物','fragment':'剧情姿态','prop-character':'场景角色'}.get(a['kind'],'相关素材')
   writer.writerow([id,'；'.join(sorted(names.get(id,names.get(base,{'未命名'})))),kind,'sheets/walk_%03d.png'%base,'sources/walk_%03d.png'%base,status,r.get('height_max',''),a.get('safe_variant','')])
 print('Character catalog entries',len(ids),flush=True)

def review():
 inventory={a['id']:a for a in plan()['items']};records=[]
 for f in (ART/'records').glob('walk_*.json'):
  r=json.loads(f.read_text(encoding='utf-8'))
  if r.get('status')=='ready' and not r['standing']:records.append(r)
 records.sort(key=lambda r:r['id'])
 for start in range(0,len(records),20):
  board=Image.new('RGB',(1280,1100),(45,52,56));draw=ImageDraw.Draw(board)
  for j,r in enumerate(records[start:start+20]):
   a=inventory[r['id']];fw,fh=r['width'],r['height'];x=j%4*320;y=j//4*220
   old=Image.open(a['source']).convert('RGBA').crop((0,0,a['width'],a['height'])).resize((fw,fh),Image.Resampling.NEAREST)
   new=Image.open(ART/'sheets'/('walk_%03d.png'%r['id'])).convert('RGBA').crop((0,0,fw,fh))
   for side,cell in enumerate((old,new)):
    cell.thumbnail((150,192),Image.Resampling.NEAREST);board.paste(cell,(x+side*160+(160-cell.width)//2,y+24),cell)
   draw.text((x+4,y+4),'%03d ORIGINAL      GENERATED'%r['id'],fill='white')
  board.save(ART/'contacts'/('pose_review_%02d.png'%(start//20)))
 print('Special pose comparison pages',(len(records)+19)//20,flush=True)

if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('command',choices=('inputs','import','verify','contacts','review','catalog','watch'));p.add_argument('--id',type=int);a=p.parse_args()
 if a.command=='import':run(a.id)
 else:globals()[a.command]()
