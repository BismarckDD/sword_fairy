"""Compare matched map locations at final display scale, without blur filters."""
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parents[1]
BASE = ROOT/'runtime/game/hd/maps/context-tiles'
NEW = BASE/'HENS00-320'


def main():
    # Previous 640px crop contains this same 320px window in its right half.
    with Image.open(BASE/'HENS00/generated/HENS00_0_0.png') as im:
        old = im.convert('RGB').crop((im.width//2,0,im.width,im.height//2))
    with Image.open(NEW/'generated/HENS00_320_0.png') as im:
        current = im.convert('RGB').copy()
    old = old.resize((1280,1280),Image.Resampling.NEAREST)
    current = current.resize((1280,1280),Image.Resampling.NEAREST)
    crop = (128,64,768,704)
    canvas = Image.new('RGB',(1280,688),(24,24,24))
    font_path = Path('C:/Windows/Fonts/arial.ttf')
    font = ImageFont.truetype(str(font_path),20) if font_path.exists() else ImageFont.load_default()
    draw = ImageDraw.Draw(canvas)
    draw.text((12,12),'Previous: 640px input, native AI ~2x',font=font,fill='white')
    draw.text((652,12),'Current: 320px input, native AI ~4x',font=font,fill='white')
    canvas.paste(old.crop(crop),(0,48))
    canvas.paste(current.crop(crop),(640,48))
    canvas.save(NEW/'previews/640-vs-320-native-detail.png')
    # Optional final-library comparison at an exact 1:1 output-pixel scale.
    final = NEW/'reconstructed-shared-tiles-4x.png'
    full = NEW/'context-redraw-4x.png'
    if final.exists() and full.exists():
        with Image.open(BASE/'HENS00/context-redraw-4x.png') as im:
            previous_map = im.convert('RGB')
        with Image.open(full) as im:
            direct_map = im.convert('RGB')
        with Image.open(final) as im:
            shared_map = im.convert('RGB')
        # One 128-source-pixel patch of building, one of trees/rocks, each
        # displayed as its actual 512px output, without resizing or sharpening.
        rows = [(1440,128,1952,640),(128,1472,640,1984)]
        out = Image.new('RGB',(1536,1080),(24,24,24))
        d = ImageDraw.Draw(out)
        for i,label in enumerate(('Previous 640px method','New 320px direct assembly','New 320px shared tiles')):
            d.text((i*512+10,12),label,font=font,fill='white')
        for y,box in enumerate(rows):
            for x,im in enumerate((previous_map,direct_map,shared_map)):
                out.paste(im.crop(box),(x*512,48+y*512))
        out.save(NEW/'previews/final-100-percent-detail.png')


if __name__ == '__main__':
    main()
