"""Export same-coordinate 1:1 details: original/baseline, direct, shared tiles."""
import argparse
import json
from pathlib import Path
from PIL import Image,ImageDraw,ImageFont

Image.MAX_IMAGE_PIXELS=300_000_000


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--work',type=Path,required=True)
    p.add_argument('--baseline',type=Path)
    p.add_argument('--region',action='append',help='Original pixel x,y,width,height; repeat for several details')
    args=p.parse_args()
    plan=json.loads((args.work/'plan.json').read_text(encoding='utf-8'))
    w,h=plan['source_size']
    with Image.open(args.work/'context-redraw-4x.png') as im: direct=im.convert('RGB')
    with Image.open(args.work/'reconstructed-shared-tiles-4x.png') as im: shared=im.convert('RGB')
    if args.baseline:
        with Image.open(args.baseline) as im: baseline=im.convert('RGB')
        assert baseline.size==direct.size, 'Baseline must have the same map coordinates and output dimensions'
    else:
        with Image.open(plan['original_source']) as im: baseline=im.convert('RGB').resize(direct.size,Image.Resampling.NEAREST)
    regions=[tuple(map(int,r.split(','))) for r in args.region] if args.region else [(0,0,min(128,w),min(128,h))]
    for x,y,rw,rh in regions:
        assert len((x,y,rw,rh))==4 and min(x,y)>=0 and min(rw,rh)>0 and x+rw<=w and y+rh<=h
    panel=max(r[2] for r in regions)*4
    out=Image.new('RGB',(panel*3,48+sum(r[3]*4 for r in regions)),(24,24,24))
    fontfile=Path('C:/Windows/Fonts/arial.ttf')
    font=ImageFont.truetype(str(fontfile),18) if fontfile.exists() else ImageFont.load_default()
    draw=ImageDraw.Draw(out)
    for col,label in enumerate(('Baseline' if args.baseline else 'Original pixels','AI context assembly','Shared 64px tiles')):
        draw.text((col*panel+8,12),label,font=font,fill='white')
    top=48
    for x,y,rw,rh in regions:
        box=(x*4,y*4,(x+rw)*4,(y+rh)*4)
        for col,im in enumerate((baseline,direct,shared)):
            out.paste(im.crop(box),(col*panel,top))
        top+=rh*4
    (args.work/'previews').mkdir(exist_ok=True)
    out.save(args.work/'previews/100-percent-detail.png')
    print(str(args.work/'previews/100-percent-detail.png'))


if __name__=='__main__': main()
