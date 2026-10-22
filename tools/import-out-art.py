"""Install supplied standalone PNG artwork into the existing local HD pack."""
from pathlib import Path
from datetime import datetime
import argparse
import hashlib
import json
import shutil
import struct

import numpy as np
from PIL import Image

ROOT = Path(__file__).resolve().parents[1]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def bmp_bytes(image):
    rgba = np.asarray(image.convert('RGBA')).copy()
    pixels = rgba[:, :, [2, 1, 0, 3]].tobytes()
    w, h = image.size
    return (b'BM' + struct.pack('<III', 54 + len(pixels), 0, 54)
            + struct.pack('<IiiHHIIiiii', 40, w, -h, 1, 32, 0,
                          len(pixels), 0, 0, 0, 0) + pixels)


def install(source, game):
    hd = game / 'hd'
    work = hd / 'imports' / ('out-' + datetime.now().strftime('%Y%m%d-%H%M%S-%f'))
    work.mkdir(parents=True)
    manifests = {p: p.read_text(encoding='utf-8-sig').splitlines()
                 for p in (hd / 'manifest.tsv', hd / 'superres/manifest.tsv')}
    entries = {}
    for path, lines in manifests.items():
        for i, line in enumerate(lines):
            if line and not line.startswith('#'):
                fields = line.split('\t')
                entries[fields[0]] = (path, i, fields)
    jobs = []
    for png in sorted(source.glob('upscale_*.png')):
        name = png.stem.removeprefix('upscale_')
        image = Image.open(png).convert('RGB')
        if name == 'BOX':
            # BOX has no palette tail: twelve plane-major 16x16 tiles.
            raw = (game / 'BOX.IMG').read_bytes()
            assert len(raw) == 16 + 12 * 256 and raw[:4] == b'IMGV'
            rgb = np.asarray(image)
            ys, xs = np.where(rgb.min(axis=2) < 220)
            crop = (int(xs.min()), 0, int(xs.max()) + 1, image.height)
            strip = image.crop(crop).resize((64, 768), Image.Resampling.LANCZOS)
            for tile in range(12):
                idx = np.frombuffer(raw[16+tile*256:16+(tile+1)*256], np.uint8)
                idx = idx.reshape(4, 16, 4).transpose(1, 2, 0).copy().tobytes()
                key = 'TILE_' + hashlib.sha256(idx).hexdigest()[:24]
                jobs.append((png, key, strip.crop((0, tile*64, 64, (tile+1)*64)), crop))
        else:
            key = ('WPS97' if name == 'WPS7' else name) + '_IMG_000'
            jobs.append((png, key, image, None))
    if not jobs:
        raise ValueError('No supported PNG inputs')
    records = []
    staged = []
    for png, key, image, crop in jobs:
        path, line, fields = entries[key]
        w, h = map(int, fields[1:3])
        base = path.parent
        if fields[3].startswith('@'):
            offset = int(fields[3][1:])
            with (base / 'references.bin').open('rb') as f:
                f.seek(offset)
                reference = f.read(768 + w*h)
        else:
            reference = (base / fields[3]).read_bytes()
        assert len(reference) == 768 + w*h
        image = image.resize((w*4, h*4), Image.Resampling.LANCZOS).convert('RGBA')
        rgba = np.asarray(image).copy()
        mask = np.frombuffer(reference[768:], np.uint8).reshape(h, w) != 255
        rgba[:, :, 3] = np.repeat(np.repeat(mask, 4, axis=0), 4, axis=1)*255
        image = Image.fromarray(rgba)
        stage = work / 'staging' / (key + '.bmp')
        stage.parent.mkdir(exist_ok=True)
        stage.write_bytes(bmp_bytes(image))
        preview = work / 'previews' / (key + '.png')
        preview.parent.mkdir(exist_ok=True)
        image.save(preview)
        with Image.open(stage) as check:
            assert check.size == (w*4, h*4)
            assert np.array_equal(np.asarray(check.convert('RGB')), rgba[:, :, :3])
        destination = base / fields[4]
        records.append(dict(source=str(png), source_sha256=sha(png), key=key,
                            target=str(destination), size=list(image.size), crop=crop,
                            previous_sha256=sha(destination), installed_sha256=sha(stage),
                            reference_sha256=hashlib.sha256(reference).hexdigest()))
        staged.append((stage, destination))
        manifests[path][line] = '\t'.join(fields[:5] + ['truecolor'])
    # All conversions and validation finish before the active files change.
    for path in manifests:
        backup = work / 'backup' / path.relative_to(hd)
        backup.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(path, backup)
    for _, destination in staged:
        backup = work / 'backup' / destination.relative_to(hd)
        backup.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(destination, backup)
    try:
        for stage, destination in staged:
            shutil.copy2(stage, destination)
        for path, lines in manifests.items():
            path.write_text('\n'.join(lines) + '\n', encoding='utf-8')
        for record in records:
            assert sha(Path(record['target'])) == record['installed_sha256']
    except Exception:
        for path in list(manifests) + [destination for _, destination in staged]:
            shutil.copy2(work / 'backup' / path.relative_to(hd), path)
        raise
    report = dict(source=str(source), inputs=len(set(r['source'] for r in records)),
                  installed=len(records), records=records)
    (work / 'report.json').write_text(json.dumps(report, ensure_ascii=False, indent=2), encoding='utf-8')
    print(json.dumps(dict(inputs=report['inputs'], installed=len(records), report=str(work / 'report.json'))))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('source', type=Path)
    parser.add_argument('--game', type=Path, default=ROOT / 'runtime/game')
    args = parser.parse_args()
    install(args.source.resolve(), args.game.resolve())
