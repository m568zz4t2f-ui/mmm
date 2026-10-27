#!/usr/bin/env python3
"""Install validated native bundles for the current user, keeping a rollback copy."""
from pathlib import Path
import datetime
import os
import plistlib
import shutil
import subprocess
import sys
import tempfile

if sys.platform != 'darwin':
    raise SystemExit('Run on macOS after building and testing.')
root = Path(__file__).resolve().parent.parent
art = root / 'build-mac/WolfTide_artefacts/Release'
home = Path.home()
targets = [
    (art/'AU/Wolf Tide.component', home/'Library/Audio/Plug-Ins/Components/Wolf Tide.component'),
    (art/'VST3/Wolf Tide.vst3', home/'Library/Audio/Plug-Ins/VST3/Wolf Tide.vst3'),
    (art/'Standalone/Wolf Tide.app', home/'Applications/Wolf Audio/Wolf Tide.app'),
]
backup = root/'install-backups'/datetime.datetime.now().strftime('%Y%m%d-%H%M%S')
backup.mkdir(parents=True, exist_ok=False)
staged, changed = [], []
try:
    for source, target in targets:
        with (source/'Contents/Info.plist').open('rb') as stream:
            version = plistlib.load(stream)['CFBundleShortVersionString']
        if version != '1.1.0':
            raise RuntimeError('Unexpected version: '+str(source))
        subprocess.run(['codesign','--force','--sign','-',str(source)], check=True)
        target.parent.mkdir(parents=True, exist_ok=True)
        temp = Path(tempfile.mkdtemp(prefix='.tide-update-', dir=target.parent))
        staged.append(temp)
        bundle = temp/target.name
        subprocess.run(['ditto',str(source),str(bundle)], check=True)
        subprocess.run(['codesign','--verify','--strict',str(bundle)], check=True)
    # Stage and verify every format before replacing any installed format.
    for (_, target), temp in zip(targets, staged):
        old = backup/target.name
        if target.exists():
            os.rename(target, old)
        changed.append((target, old))
        os.rename(temp/target.name, target)
    for _, target in targets:
        subprocess.run(['codesign','--verify','--strict',str(target)], check=True)
        print('Installed 1.1.0:', target)
    print('Previous versions:', backup)
except Exception:
    for target, old in reversed(changed):
        if target.exists():
            shutil.rmtree(target)
        if old.exists():
            os.rename(old, target)
    raise
finally:
    for temp in staged:
        if temp.exists():
            shutil.rmtree(temp)
