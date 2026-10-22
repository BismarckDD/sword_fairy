"""Reuse a native redraw only when its original crop SHA-256 is identical."""
import hashlib
import json
from pathlib import Path
import subprocess
import sys

root=Path(__file__).resolve().parents[1]
maps=root/'runtime/game/hd/maps'
scope=json.loads((maps/'imagegen/linan-scope.json').read_text(encoding='utf-8'))
names={s['scene'] for s in scope['scenes']}
plans=[]
cache={}
for path in sorted((maps/'context-tiles').glob('*-320/plan.json')):
    p=json.loads(path.read_text(encoding='utf-8'))
    if p['scene'] not in names:
        continue
    plans.append((path,p))
    for job in p['jobs']:
        if job['status']=='generated':
            image=path.parent/job['generated']
            assert hashlib.sha256(image.read_bytes()).hexdigest()==job['generated_sha256']
            cache[job['source_sha256']]=(image,path,job['id'])
count=0
script=Path.home()/'.codex/skills/game-map-redraw/scripts/map_redraw.py'
for path,plan in plans:
    records={}
    recordpath=path.parent/'reuse-provenance.json'
    if recordpath.exists():
        records=json.loads(recordpath.read_text(encoding='utf-8'))
    for job in plan['jobs']:
        if job['status']=='generated' or job['source_sha256'] not in cache:
            continue
        image,origin,origin_job=cache[job['source_sha256']]
        subprocess.run([sys.executable,str(script),'register','--project-root',str(root),
                        '--scene',plan['scene'],'--job',job['id'],'--source',str(image)],check=True,stdout=subprocess.DEVNULL)
        records[job['id']]=dict(original_source_sha256=job['source_sha256'],origin_plan=str(origin),
                                origin_job=origin_job,native_image=str(image),
                                prompts=str(origin.parent/'prompts.txt'),exact_original_crop_reuse=True)
        recordpath.write_text(json.dumps(records,indent=2),encoding='utf-8')
        count+=1
print(json.dumps(dict(reused_identical_original_crops=count)))
