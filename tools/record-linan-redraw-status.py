"""Refresh resumable Linan art status from real plans and verification reports."""
import json
import hashlib
from pathlib import Path

root=Path(__file__).resolve().parents[1]
maps=root/'runtime/game/hd/maps'
art=maps/'imagegen'
scope=json.loads((art/'linan-scope.json').read_text(encoding='utf-8'))
rows=[r for r in (art/'scene-manifest.tsv').read_text(encoding='utf-8').splitlines() if r and not r.startswith('#')]
for scene in scope['scenes']:
    name=scene['scene']
    work=maps/f'context-tiles/{name}-320'
    plan=work/'plan.json'
    if plan.exists():
        p=json.loads(plan.read_text(encoding='utf-8'))
        scene.update(context_jobs=len(p['jobs']),registered_jobs=sum(j['status']=='registered' for j in p['jobs']))
        # Plans use generated as their persisted status after registration.
        scene['registered_jobs']=sum(bool(j.get('generated') or j.get('generated_sha256') or j['status'] in ('registered','generated')) for j in p['jobs'])
    report=art/f'verification-reports/{name}-context320-installation.json'
    if report.exists():
        r=json.loads(report.read_text(encoding='utf-8'))
        scene.update(status=r['status'],installation_report=str(report.relative_to(art)),
                     assembled=str((work/'context-redraw-4x.png').relative_to(maps)),native_AI_scales=r['native_AI_scales'])
    else:
        scene['status']='generating' if scene.get('registered_jobs',0) else 'pending-context-redraw'
        previous=art/f'scene-verification/{name}.json'
        if name in ('LAN00','FBT00','JHY','JJF') and previous.exists():
            r=json.loads(previous.read_text(encoding='utf-8'))
            payload='\n'.join(row for row in rows if row.split('\t')[0]==name)+'\n'
            if r.get('scene_manifest_sha256')==hashlib.sha256(payload.encode()).hexdigest():
                scene.update(status='retained-existing-resource-verified',existing_verification=str(previous.relative_to(art)))
scope['completed_context_scenes']=[s['scene'] for s in scope['scenes'] if s['status']=='installed-and-native-verified']
scope['all_context_scenes_complete']=len(scope['completed_context_scenes'])==len(scope['scenes'])
scope['retained_existing_scenes']=[s['scene'] for s in scope['scenes'] if s['status']=='retained-existing-resource-verified']
scope['all_scope_scenes_installed']=len(scope['completed_context_scenes'])+len(scope['retained_existing_scenes'])==len(scope['scenes'])
(art/'linan-scope.json').write_text(json.dumps(scope,ensure_ascii=False,indent=2),encoding='utf-8')
print(json.dumps(dict(completed=scope['completed_context_scenes'],all_complete=scope['all_context_scenes_complete'],
                      retained=scope['retained_existing_scenes'],
                      active=[dict(scene=s['scene'],registered=s.get('registered_jobs',0),total=s.get('context_jobs',0))
                              for s in scope['scenes'] if s.get('registered_jobs',0) and s['status']!='installed-and-native-verified']),ensure_ascii=False))
