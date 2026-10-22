"""Finish, verify, install and optionally run one context-map art probe."""
import argparse
from pathlib import Path
import subprocess
import sys

parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--scene',required=True)
parser.add_argument('--region',default='0,0,128,128')
parser.add_argument('--place',type=int)
parser.add_argument('--x',type=int)
parser.add_argument('--y',type=int)
args=parser.parse_args()
root=Path(__file__).resolve().parents[1]
skill=Path.home()/'.codex/skills/game-map-redraw/scripts'
def run(script,*arguments):
    subprocess.run([sys.executable,str(script),*map(str,arguments)],cwd=root,check=True)
for step in ('finish','verify'):
    extra=['--allow-near4x'] if step=='finish' else []
    run(skill/'map_redraw.py',step,'--project-root',root,'--scene',args.scene,*extra)
run(skill/'compare.py','--work',root/f'runtime/game/hd/maps/context-tiles/{args.scene}-320','--region',args.region)
run(root/'tools/install-context-map.py','--scene',args.scene)
if args.place is not None:
    assert args.x is not None and args.y is not None
    run(root/'tools/probe-context-map.py','--scene',args.scene,'--place',args.place,'--x',args.x,'--y',args.y)
