"""Import an existing approved 5x4 handpainted hero sheet; no image generation.
Crops, uniform resampling and foot registration preserve the supplied artwork.
"""
from pathlib import Path
import argparse,hashlib,json,struct,shutil
import numpy as np
from PIL import Image
ROOT=Path(__file__).resolve().parents[1];WORK=ROOT/'runtime/handpaint-walk';GAME=ROOT/'runtime/game'
def main(source):
 WORK.mkdir(parents=True,exist_ok=True);source=Path(source);shutil.copy2(source,WORK/'approved-source.png')
 image=Image.open(source).convert('RGBA');npc=(GAME/'NPC.DAT').read_bytes();at=struct.unpack_from('<I',(GAME/'NPC.IDX').read_bytes(),164*4)[0]
 w,h,nf,af,nd,ad=struct.unpack_from('<6H',npc,at);assert (w,h,nf,af,nd)==(1,3,5,0,4)
 order=list(struct.unpack_from('<60h',npc,at+18));order=[abs(i)&4095 for i in order];assert sorted(order)==list(range(60))
 # The approved drawing has uneven row spacing. Use the actual transparent
 # gutters rather than cutting through a head at exact quarter-height.
 occupied=(np.asarray(image)[:,:,3]>32).sum(1);gaps=[];start=None
 for y,count in enumerate(occupied):
  if count<5 and start is None:start=y
  if count>=5 and start is not None:
   if y-start>8 and start>image.height//8:gaps.append((start,y))
   start=None
 assert len(gaps)>=3
 cuts=[0]+[sum(min(gaps,key=lambda g:abs((g[0]+g[1])/2-image.height*r/4)))//2 for r in (1,2,3)]+[image.height]
 assert cuts==sorted(set(cuts))
 frames=[];boxes=[]
 for row in range(4):
  for col in range(5):
   frame=image.crop((col*image.width//5,cuts[row],(col+1)*image.width//5,cuts[row+1]))
   alpha=np.asarray(frame)[:,:,3];yy,xx=np.where(alpha>32);assert len(xx)
   box=(int(xx.min()),int(yy.min()),int(xx.max())+1,int(yy.max())+1);frames.append(frame);boxes.append(box)
 scale=min(60/max(b[2]-b[0] for b in boxes),180/max(b[3]-b[1] for b in boxes))
 sheet=Image.new('RGBA',(320,768));tiles=[None]*60;original=Image.open(ROOT/'runtime/ai-redraw/walk-source/walk_164.png').convert('RGBA')
 for k,(frame,box) in enumerate(zip(frames,boxes)):
  row,col=divmod(k,5);body=frame.crop(box);body=body.resize((max(1,round(body.width*scale)),max(1,round(body.height*scale))),Image.Resampling.LANCZOS)
  old=np.asarray(original.crop((col*16,row*48,(col+1)*16,(row+1)*48)))[:,:,3];ys,xs=np.where(old>128);foot=(int(ys.max())+1)*4
  cell=Image.new('RGBA',(64,192));cell.alpha_composite(body,((64-body.width)//2,foot-body.height));sheet.alpha_composite(cell,(col*64,row*192))
  for ty in range(3):tiles[order[k*3+ty]]=np.asarray(cell.crop((0,ty*64,64,(ty+1)*64)))[:,:,[2,1,0,3]].tobytes()
 data=b'JXWALK1\0'+struct.pack('<I',60)+(GAME/'COMMON.PAL').read_bytes()+b''.join(tiles)
 (WORK/'walk_164.bin').write_bytes(data);sheet.save(WORK/'registered-sheet.png')
 preview=Image.new('RGBA',sheet.size,(46,55,59,255));preview.alpha_composite(sheet);preview.convert('RGB').save(WORK/'registered-preview.png')
 report=dict(resource=164,actor=0,frames=20,directions=4,frames_per_direction=5,source_sha256=hashlib.sha256(source.read_bytes()).hexdigest(),output_sha256=hashlib.sha256(data).hexdigest(),uniform_scale=scale,render_cell=[64,192],alpha='supplied artwork alpha; not clipped to the old indexed silhouette',foot_registration='original per-frame bottom',source_archives_modified=False)
 (WORK/'import.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8');print(json.dumps(report))
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('source');main(p.parse_args().source)
