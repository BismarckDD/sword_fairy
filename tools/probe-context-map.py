"""Run a bounded hidden native art probe with isolated output and no save writes."""
import argparse
import ctypes
from ctypes import wintypes
import hashlib
import json
from pathlib import Path
import re
import shutil
import subprocess
import time
from PIL import Image

parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--scene',required=True)
parser.add_argument('--place',required=True,type=int)
parser.add_argument('--x',required=True,type=int)
parser.add_argument('--y',required=True,type=int)
parser.add_argument('--executable',type=Path)
parser.add_argument('--variant',default='')
args=parser.parse_args()
root=Path(__file__).resolve().parents[1]
game=root/'runtime/game'
art=game/'hd/maps/imagegen'
fixture=root/'build/linan-context-validation'
dest=art/f'verification-reports/{args.scene}-context320-native{args.variant}'
dest.mkdir(exist_ok=True)
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
userdata=game/'main_decompiled/userdata'
saves={str(p.relative_to(userdata)):sha(p) for p in userdata.rglob('*') if p.is_file()}
startup=subprocess.STARTUPINFO()
startup.dwFlags|=subprocess.STARTF_USESHOWWINDOW
startup.wShowWindow=0
user32=ctypes.WinDLL('user32',use_last_error=True)
callback_type=ctypes.WINFUNCTYPE(wintypes.BOOL,wintypes.HWND,wintypes.LPARAM)
user32.EnumWindows.argtypes=[callback_type,wintypes.LPARAM]
user32.GetWindowThreadProcessId.argtypes=[wintypes.HWND,ctypes.POINTER(wintypes.DWORD)]
user32.SendMessageTimeoutW.argtypes=[wintypes.HWND,wintypes.UINT,wintypes.WPARAM,wintypes.LPARAM,wintypes.UINT,wintypes.UINT,ctypes.POINTER(ctypes.c_size_t)]
with (dest/'probe.log').open('w',encoding='utf-8') as log:
    process=subprocess.Popen([str(args.executable.resolve() if args.executable else fixture/'probe.exe'),str(args.place),'--play','64','0',str(args.x),str(args.y)],
                             cwd=game,stdout=log,stderr=subprocess.STDOUT,startupinfo=startup)
    def paint(hwnd,_):
        owner=wintypes.DWORD()
        user32.GetWindowThreadProcessId(hwnd,ctypes.byref(owner))
        if owner.value==process.pid:
            result=ctypes.c_size_t()
            user32.SendMessageTimeoutW(hwnd,0x000f,0,0,2,50,ctypes.byref(result))
        return True
    callback=callback_type(paint)
    deadline=time.monotonic()+120
    while process.poll() is None:
        if time.monotonic()>deadline:
            process.kill()
            process.wait()
            raise TimeoutError('Bounded native art probe timed out')
        user32.EnumWindows(callback,0)
        time.sleep(.04)
text=(dest/'probe.log').read_text(encoding='utf-8',errors='replace')
assert process.returncode==0,text[-5000:]
match=re.search(r'CONTEXT MAP HIT: map=(\S+) cells=(\d+) frames=(\d+)',text)
assert match and match[1].upper()==args.scene and int(match[2])>0,text[-5000:]
frames = int(match[3])
# A queued timer can complete one extra frame before the window closes.
assert 64 <= frames <= 65
assert saves=={str(p.relative_to(userdata)):sha(p) for p in userdata.rglob('*') if p.is_file()}
shutil.copy2(fixture/'analysis/preview.bmp',dest/'preview.bmp')
Image.open(dest/'preview.bmp').save(dest/'preview.png')
record=art/f'verification-reports/{args.scene}-context320-installation.json'
report=json.loads(record.read_text(encoding='utf-8'))
if args.variant:
    record=dest/'verification.json'
report.update(status='installed-and-native-verified',native_probe_completed=True,
              map_cells=int(match[2]),world_frames=frames,saves_unchanged=True,
              native_preview=str(dest/'preview.png'),native_log=str(dest/'probe.log'),
              test_scope='Isolated original renderer; fixture actor coordinates; entry dialog waits bypassed; no full-story acceptance')
record.write_text(json.dumps(report,indent=2),encoding='utf-8')
print(json.dumps(report))
