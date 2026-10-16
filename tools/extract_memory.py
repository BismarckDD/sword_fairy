"""Isolated real-mode loader investigation; no host program execution."""
import sys, struct
from pathlib import Path
from unicorn import *
from unicorn.x86_const import *
from capstone import *
Path('save_tool').mkdir(exist_ok=True)
b=Path('MAIN.EXE').read_bytes()
u=Uc(UC_ARCH_X86,UC_MODE_16);u.mem_map(0,0x200000)
base=0x10000
u.mem_write(base,b[0x2800:0x2f4da])
u.mem_write(0xff00,b'\xcd\x20');u.mem_write(0xff02,struct.pack('<H',0x9000))
u.mem_write(0xff16,struct.pack('<H',0xff0))
u.mem_write(0xff2c,struct.pack('<H',0x900));u.mem_write(0x9000,b'PATH=C:\\GAME\x00\x00\x01\x00C:\\GAME\\MAIN.EXE\x00')
for i in range(256):u.mem_write(i*4,struct.pack('<HH',i*4,0xf000));u.mem_write(0xf0000+i*4,bytes([0xcd,i,0xcf,0x90]))
for r,v in [(UC_X86_REG_CS,0x1000),(UC_X86_REG_IP,0x200),(UC_X86_REG_DS,0xff0),(UC_X86_REG_ES,0xff0),(UC_X86_REG_SS,0x1000),(UC_X86_REG_SP,0xc0)]:u.reg_write(r,v)
def reg(r):return u.reg_read(r)
def carry(v):
 u.reg_write(UC_X86_REG_EFLAGS,(reg(UC_X86_REG_EFLAGS)&~1)|v)
 if reg(UC_X86_REG_CS)==0xf000:
  a=reg(UC_X86_REG_SS)*16+reg(UC_X86_REG_SP)+4;f=struct.unpack('<H',u.mem_read(a,2))[0];u.mem_write(a,struct.pack('<H',(f&~1)|v))
handles={}; next_handle=5
def intr(u,n,_):
 global next_handle
 ax=reg(UC_X86_REG_AX); ah=ax>>8;al=ax&255
 if n!=3: print('INT',hex(n),hex(ax),'at',hex(reg(UC_X86_REG_CS)),hex(reg(UC_X86_REG_IP)),flush=True)
 if n==0x21 and reg(UC_X86_REG_CS)!=0xf000 and struct.unpack('<H',u.mem_read(0x86,2))[0]!=0xf000:
  off,seg=struct.unpack('<HH',u.mem_read(n*4,4))
  for val in [reg(UC_X86_REG_EFLAGS)&65535,reg(UC_X86_REG_CS),reg(UC_X86_REG_IP)]:
   sp=(reg(UC_X86_REG_SP)-2)&65535;u.reg_write(UC_X86_REG_SP,sp);u.mem_write(reg(UC_X86_REG_SS)*16+sp,struct.pack('<H',val))
  u.reg_write(UC_X86_REG_CS,seg);u.reg_write(UC_X86_REG_IP,off)
 elif n==0x21:
  if ah==0x30:u.reg_write(UC_X86_REG_AX,5)
  elif ah==0x35:
   off,seg=struct.unpack('<HH',u.mem_read(al*4,4));u.reg_write(UC_X86_REG_BX,off);u.reg_write(UC_X86_REG_ES,seg)
  elif ah==0x25:u.mem_write(al*4,struct.pack('<HH',reg(UC_X86_REG_DX),reg(UC_X86_REG_DS)))
  elif ah==0x3d:
   name=bytes(u.mem_read(reg(UC_X86_REG_DS)*16+reg(UC_X86_REG_DX),128)).split(b'\0')[0];print('OPEN',name)
   if b'MAIN' in name.upper():handles[next_handle]=0;u.reg_write(UC_X86_REG_AX,next_handle);next_handle+=1;carry(0)
   else:u.reg_write(UC_X86_REG_AX,2);carry(1)
  elif ah==0x3f:
   h=reg(UC_X86_REG_BX);pos=handles.get(h,0);d=b[pos:pos+reg(UC_X86_REG_CX)];u.mem_write(reg(UC_X86_REG_DS)*16+reg(UC_X86_REG_DX),d);handles[h]=pos+len(d);u.reg_write(UC_X86_REG_AX,len(d));carry(0)
  elif ah==0x42:
   h=reg(UC_X86_REG_BX);p=(reg(UC_X86_REG_CX)<<16)|reg(UC_X86_REG_DX);p=p if p<2**31 else p-2**32;p+=[0,handles.get(h,0),len(b)][al];handles[h]=p;u.reg_write(UC_X86_REG_AX,p&65535);u.reg_write(UC_X86_REG_DX,p>>16);carry(0)
  elif ah==0x3e:carry(0)
  elif ah==0x52:u.reg_write(UC_X86_REG_ES,0x800);u.reg_write(UC_X86_REG_BX,2);u.mem_write(0x8000,struct.pack('<H',0xfef))
  elif ah==0x4a:carry(0)
  elif ah==0x2d:u.reg_write(UC_X86_REG_AX,0)
  elif ah==0x2c:u.reg_write(UC_X86_REG_CX,0x1200);u.reg_write(UC_X86_REG_DX,0)
  elif ah==0x2a:u.reg_write(UC_X86_REG_AX,0x2a03);u.reg_write(UC_X86_REG_CX,1997);u.reg_write(UC_X86_REG_DX,0x0101)
  elif ah==0x09:print(bytes(u.mem_read(reg(UC_X86_REG_DS)*16+reg(UC_X86_REG_DX),150)))
  elif ah==0x4c:u.emu_stop()
  else:u.emu_stop()
 elif n==0x15:u.reg_write(UC_X86_REG_AX,0);carry(0)
 elif n==0x10:u.reg_write(UC_X86_REG_AX,0x5003);u.reg_write(UC_X86_REG_BX,0)
 elif n==0x2f:u.reg_write(UC_X86_REG_AX,0)
 elif n==0x1a:u.reg_write(UC_X86_REG_CX,0);u.reg_write(UC_X86_REG_DX,1234)
 else:
  off,seg=struct.unpack('<HH',u.mem_read(n*4,4))
  for val in [reg(UC_X86_REG_EFLAGS)&65535,reg(UC_X86_REG_CS),reg(UC_X86_REG_IP)]:
   sp=(reg(UC_X86_REG_SP)-2)&65535;u.reg_write(UC_X86_REG_SP,sp);u.mem_write(reg(UC_X86_REG_SS)*16+sp,struct.pack('<H',val))
  u.reg_write(UC_X86_REG_CS,seg);u.reg_write(UC_X86_REG_IP,off)
trace=[]
def code(u,a,size,_):
 if reg(UC_X86_REG_CS)==0x3cce and reg(UC_X86_REG_IP)==0x10ad:Path('save_tool/stage.bin').write_bytes(bytes(u.mem_read(0x3cce0,0x2100)))
 if reg(UC_X86_REG_IP)==0x1ed2:u.emu_stop()
 if size>=3 and bytes(u.mem_read(a,3))==b'\x2e\xff\x26':print('DISPATCH',hex(reg(UC_X86_REG_CS)),hex(reg(UC_X86_REG_IP)),hex(reg(UC_X86_REG_SI)),hex(reg(UC_X86_REG_AX)),flush=True)
 if reg(UC_X86_REG_CS)==0x3cce and reg(UC_X86_REG_IP)==0x10a4:u.reg_write(UC_X86_REG_AX,0) # bypass environment/debugger probe in emulator only
 trace.append((reg(UC_X86_REG_CS),reg(UC_X86_REG_IP),bytes(u.mem_read(a,size)).hex()))
 if len(trace)>100:del trace[:50]
 if reg(UC_X86_REG_CS) not in (0x1000,0x3cce):
  if reg(UC_X86_REG_CS)!=0xf000:
   print('OUTSIDE',hex(a),flush=True);u.emu_stop()
ports={}
def pin(u,p,size,_):return ports.get(p,0)
def pout(u,p,size,v,_):ports[p]=v
u.hook_add(UC_HOOK_INSN,pin,None,1,0,UC_X86_INS_IN)
u.hook_add(UC_HOOK_INSN,pout,None,1,0,UC_X86_INS_OUT)
u.hook_add(UC_HOOK_INTR,intr);u.hook_add(UC_HOOK_CODE,code)
try:u.emu_start(0x10200,0x200000,count=3000000)
except UcError as e:print(e)
Path('save_tool/memory.bin').write_bytes(bytes(u.mem_read(0,0x100000)))
print('REGS',[(n,hex(reg(globals()['UC_X86_REG_'+n]))) for n in ['CS','IP','SS','SP','DS','ES','AX','BX','CX','DX','SI','DI','BP']])
print('\n'.join(map(str,trace[-35:])))
