#!/usr/bin/env python3
"""Reproducible corresponding source archive, without build products or user secrets."""
import sys, zipfile, os, gzip, subprocess, tempfile, shutil, atexit
from pathlib import Path
root=Path(__file__).resolve().parent.parent
output=Path(sys.argv[1]).resolve()
output.parent.mkdir(parents=True,exist_ok=True)
pinned='29396c22c93392d6738e021b83196283d6e4d850'
allowed={'Source','tests','docs','scripts','vendor','.github','Factory-Presets'}
handle=tempfile.NamedTemporaryFile(prefix='.tide-source-',dir=output.parent,delete=False)
staged=Path(handle.name);handle.close()
atexit.register(lambda: staged.unlink(missing_ok=True))
with zipfile.ZipFile(staged,'w',zipfile.ZIP_DEFLATED,compresslevel=6) as z:
    for p in sorted(root.rglob('*')):
        rel=p.relative_to(root)
        if not p.is_file() or p.resolve()==output or '__pycache__' in rel.parts: continue
        if rel.as_posix() in {'CMakeLists.txt','README.md','LICENSE','.gitignore'} or rel.parts[0] in allowed:
            info=zipfile.ZipInfo('WolfTide/'+rel.as_posix(),date_time=(2026,10,27,0,0,0))
            info.external_attr=(0o100755 if p.suffix in {'.sh','.command','.py'} or p.stat().st_mode & 0o111 else 0o100644)<<16
            info.compress_type=zipfile.ZIP_DEFLATED
            z.writestr(info,p.read_bytes())
# Build distributions must carry JUCE source, even when CMake fetched it.
    if not (root/'vendor/JUCE-8.0.12.tar.gz').is_file():
        candidates=[Path(os.environ['TIDE_JUCE_SOURCE'])] if os.environ.get('TIDE_JUCE_SOURCE') else [root/name/'_deps/juce-src' for name in ('build-mac','build')]
        source=next((p for p in candidates if (p/'.git').exists()),None)
        if source is None:
            raise SystemExit('Pinned JUCE source missing. Supply vendor/JUCE-8.0.12.tar.gz or set TIDE_JUCE_SOURCE to the pinned Git checkout.')
        if subprocess.check_output(['git','-C',str(source),'rev-parse','HEAD'],text=True).strip()!=pinned:
            raise SystemExit('JUCE source must match the pinned commit.')
        with tempfile.TemporaryFile() as archive:
            process=subprocess.Popen(['git','-C',str(source),'archive','--format=tar',pinned],stdout=subprocess.PIPE)
            with gzip.GzipFile(fileobj=archive,mode='wb',mtime=0) as compressed:
                shutil.copyfileobj(process.stdout,compressed)
            if process.wait()!=0:raise SystemExit('Could not archive pinned JUCE source.')
            archive.seek(0)
            info=zipfile.ZipInfo('WolfTide/vendor/JUCE-8.0.12.tar.gz',date_time=(2026,10,27,0,0,0))
            info.external_attr=0o100644<<16;info.compress_type=zipfile.ZIP_DEFLATED
            z.writestr(info,archive.read())
with staged.open('rb') as completed:os.fsync(completed.fileno())
os.replace(staged,output)
print(output)
