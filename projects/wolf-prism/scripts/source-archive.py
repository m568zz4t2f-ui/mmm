#!/usr/bin/env python3
from pathlib import Path
import sys,zipfile
root=Path(__file__).resolve().parents[1]
with zipfile.ZipFile(sys.argv[1],'w',zipfile.ZIP_DEFLATED) as z:
 for base in ['Source','scripts','tests','docs','vendor']:
  for p in (root/base).rglob('*'):
   if p.is_file():z.write(p,p.relative_to(root))
 for n in ['README.md','CMakeLists.txt','LICENSE']:z.write(root/n,n)
