"""Record Tianwang map scope from original place headers and exits."""
import hashlib
import json
import struct
from pathlib import Path

root = Path(__file__).resolve().parents[1]
game = root / 'runtime/game'
data = (game / 'PLACE.BIN').read_bytes()
selected = list(range(150, 160)) + [176]
places = []
for i in selected:
    record = data[i*2784:(i+1)*2784]
    name = record[:14].split(b'\0')[0].decode('ascii').upper()
    exits = sorted({struct.unpack_from('<8H', record, 64+j*16)[3]
                    for j in range(50) if any(record[64+j*16:80+j*16])})
    places.append(dict(place=i, scene=name, exits=exits))
scenes = []
for place in places:
    work = game / f"hd/maps/context-tiles/{place['scene']}-320"
    plan = json.loads((work/'plan.json').read_text(encoding='utf-8'))
    scenes.append(dict(scene=place['scene'], place=place['place'],
                       size=plan['source_size'], total=len(plan['jobs']),
                       generated=sum(j['status']=='generated' for j in plan['jobs']),
                       plan=str(work/'plan.json'),
                       installed=(game/f"hd/maps/imagegen/verification-reports/{place['scene']}-context320-installation.json").exists()))
report = dict(area='天王帮与洞庭湖关联地图',
              evidence='PLACE.BIN headers and exits; original restoration RESUME.md places 154..159 and 176',
              place_bin_sha256=hashlib.sha256(data).hexdigest(),
              source_places=places, scenes=scenes,
              all_installed=all(s['installed'] for s in scenes))
out = game/'hd/maps/imagegen/tianwang-scope.json'
out.write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf-8')
print(json.dumps(scenes))
