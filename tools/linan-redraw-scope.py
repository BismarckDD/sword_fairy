"""Record Linan art scope from original place headers and exits."""
import hashlib
import json
import struct
from pathlib import Path

root = Path(__file__).resolve().parents[1]
game = root / 'runtime/game'
art = game / 'hd/maps/imagegen'
data = (game / 'PLACE.BIN').read_bytes()
places = []
for i in range(len(data) // 2784):
    record = data[i*2784:(i+1)*2784]
    name = record[:14].split(b'\0')[0].decode('ascii').upper()
    exits = sorted({struct.unpack_from('<8H', record, 64+j*16)[3]
                    for j in range(50) if any(record[64+j*16:80+j*16])})
    places.append(dict(place=i, scene=name, exits=exits))
# Main-city variants and their direct urban destinations, plus connected
# tavern/inn upper floors and later-story Linan interiors. Wuyi destinations
# are recorded as borders, rather than included merely by graph reachability.
city = [5, 23, 31, 41, 205]
urban = {7, 8, 9, 10, 11, 12, 21, 26, 27, 28, 29, 32, 33, 34,
         35, 38, 42, 43, 44, 45, 206, 208, 214}
interiors = {13, 14, 15, 36, 37, 46, 47, 48, 49, 207, 209, 210, 211, 212}
selected = set(city) | urban | interiors | {4, 30, 204}
catalog = json.loads((game/'hd/maps/catalog.json').read_text(encoding='utf-8'))
names = sorted({places[i]['scene'] for i in selected})
scenes = []
for name in names:
    s = next((s for s in catalog['scenes'] if s['name'] == name), None)
    if not s:
        continue
    scenes.append(dict(scene=name, places=[p['place'] for p in places if p['scene']==name],
                       size=[s['width']*16,s['height']*16],
                       context_plan=f'../context-tiles/{name}-320/plan.json'))
report = dict(area='临安城及关联场景', source='PLACE.BIN headers and exits',
              place_bin_sha256=hashlib.sha256(data).hexdigest(),
              source_places=[places[i] for i in sorted(selected)], scenes=scenes,
              existing_main_city='LAN00 already installed; retain until replacement verified',
              excluded=[dict(scene='FSD',reason='No city/interior exit connection; place 213 adjacency alone is not geographic evidence'),
                        dict(scene='LANTL02/LANTL04',reason='Referenced from place 71 JZF via 73..75, not Linan city; Linan later-story jail uses LANTL00/LANTL06 at 206/207')],
              status='in progress; individual context plans are authoritative')
(art/'linan-scope.json').write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf-8')
print(json.dumps([{'scene':s['scene'],'size':s['size']} for s in scenes]))
