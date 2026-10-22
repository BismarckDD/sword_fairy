"""Prepare an isolated renderer probe; never patch the shipping executable."""
from pathlib import Path

root = Path(__file__).resolve().parents[1]
out = root/'build/linan-context-validation'
(out/'build').mkdir(parents=True,exist_ok=True)
(out/'analysis').mkdir(exist_ok=True)
s = (root/'src/platform/windows/resource_host.c').read_text(encoding='utf-8')
for folder in ('build','analysis'):
    s = s.replace(f'main_decompiled/{folder}/', f'{out.as_posix()}/{folder}/')
needle = '    if(jx_map_load(&h->c,(uint16_t)place,0,(JxFarPointer){0x300,0x9000}))fail(h,"map load terminated");'
assert s.count(needle) == 1
s = s.replace(needle, '''    h->story_probe=1;
'''+needle+'''
    if(argc>6) {
        for(unsigned actor=0;actor<3;actor++) {
            jx_set_word(&h->c,(uint16_t)(0x8760+12*actor),(uint16_t)strtoul(argv[5],NULL,10));
            jx_set_word(&h->c,(uint16_t)(0x8762+12*actor),(uint16_t)strtoul(argv[6],NULL,10));
        }
    }
''')
needle = '    printf("native resource scene PASS:'
i = s.index(needle)
s = s[:i]+f'''
    if(play && h->window.hd) {{
        printf("CONTEXT MAP HIT: map=%.12s cells=%u frames=%u\\n",jx_ds(&h->c)+0x9356,jx_hd_map_hits(h->window.hd),jx_word(&h->c,0x585e));
        if(!jx_hd_export_bmp(h->window.hd,h->video,h->palette,"{out.as_posix()}/analysis/preview.bmp"))fail(h,"map preview export failed");
    }}
'''+s[i:]
(out/'resource_host_context.c').write_text(s,encoding='utf-8')
print(out)
