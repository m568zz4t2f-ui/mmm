#!/usr/bin/env python3
"""Reproducible corresponding source archive, without build products or user secrets."""
import sys, zipfile
from pathlib import Path
root=Path(__file__).resolve().parent.parent
output=Path(sys.argv[1]).resolve()
output.parent.mkdir(parents=True,exist_ok=True)
allowed={'Source','tests','docs','scripts','vendor','.github','Factory-Presets'}
with zipfile.ZipFile(output,'w',zipfile.ZIP_DEFLATED,compresslevel=6) as z:
    for p in sorted(root.rglob('*')):
        rel=p.relative_to(root)
        if not p.is_file() or p.resolve()==output or '__pycache__' in rel.parts: continue
        if rel.as_posix() in {'CMakeLists.txt','README.md','LICENSE','.gitignore'} or rel.parts[0] in allowed:
            info=zipfile.ZipInfo('WolfTide/'+rel.as_posix(),date_time=(2026,10,27,0,0,0))
            info.external_attr=(0o100755 if p.suffix in {'.sh','.command','.py'} else 0o100644)<<16
            info.compress_type=zipfile.ZIP_DEFLATED
            z.writestr(info,p.read_bytes())
print(output)
