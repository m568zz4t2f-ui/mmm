#!/usr/bin/env python3
"""Inspect actual universal payloads and corresponding source; emit CI evidence."""
import hashlib
import json
import plistlib
import re
import subprocess
import tempfile
import zipfile
from pathlib import Path

root = Path(__file__).resolve().parents[1]
dist = root / 'dist'
version = re.search(r'project\(WolfTide VERSION ([0-9.]+)', (root/'CMakeLists.txt').read_text())[1]
build_id = re.search(r'TIDE_BUILD_ID "([a-f0-9]+)"', (root/'build-mac/TideBuild.h').read_text())[1]
report = dict(version=version, native_build_id=build_id,
              source_revision=subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=root, text=True).strip(),
              runner_architecture=subprocess.check_output(['uname', '-m'], text=True).strip(),
              os=subprocess.check_output(['sw_vers'], text=True),
              installer_signature='unsigned', notarization='not submitted', bundles=[])
staging = tempfile.TemporaryDirectory(prefix='tide-inspection-')
for kind, suffix in [('AU', 'component'), ('VST3', 'vst3'), ('Standalone', 'app')]:
    destination = Path(staging.name)/kind
    subprocess.run(['ditto', '-x', '-k', str(dist/f'Wolf-Tide-{version}-{kind}.zip'), str(destination)], check=True)
    bundle = destination/f'Wolf Tide.{suffix}'
    info = plistlib.loads((bundle/'Contents/Info.plist').read_bytes())
    assert info['CFBundleShortVersionString'] == version, info
    binary = bundle/'Contents/MacOS'/info['CFBundleExecutable']
    subprocess.run(['lipo', str(binary), '-verify_arch', 'arm64', 'x86_64'], check=True)
    slices = {}
    for arch in ['arm64', 'x86_64']:
        load = subprocess.check_output(['otool', '-arch', arch, '-l', str(binary)], text=True)
        minimum = re.search(r'cmd LC_BUILD_VERSION.*?\n\s+minos (\S+)', load, re.S)
        if minimum is None:
            minimum = re.search(r'cmd LC_VERSION_MIN_MACOSX.*?\n\s+version (\S+)', load, re.S)
        assert minimum, f'Missing deployment metadata: {kind}/{arch}'
        assert tuple(map(int, minimum[1].split('.')))[:2] == (11, 0), minimum[1]
        slices[arch] = dict(minimum_macos=minimum[1])
    subprocess.run(['codesign', '--verify', '--strict', str(bundle)], check=True)
    signature = subprocess.run(['codesign', '-d', '--verbose=4', str(bundle)], check=True, capture_output=True, text=True).stderr
    assert 'Signature=adhoc' in signature, signature
    report['bundles'].append(dict(format=kind, bundle_id=info['CFBundleIdentifier'],
                                  version=info['CFBundleShortVersionString'], architectures=slices,
                                  executable_sha256=hashlib.sha256(binary.read_bytes()).hexdigest(),
                                  signature=signature))

source = dist/f'Wolf-Tide-{version}-Source.zip'
with zipfile.ZipFile(source) as archive:
    names = set(archive.namelist())
    allowed = {'Source', 'tests', 'docs', 'scripts', 'vendor', 'assets', '.github', 'Factory-Presets'}
    tracked = subprocess.check_output(['git', 'ls-files', '-z'], cwd=root).decode().split('\0')
    for name in filter(None, tracked):
        if name in {'CMakeLists.txt', 'README.md', 'LICENSE', '.gitignore'} or name.split('/')[0] in allowed:
            assert 'WolfTide/'+name in names, 'Source archive missing '+name
            assert archive.read('WolfTide/'+name) == (root/name).read_bytes(), 'Source mismatch '+name
    assert 'WolfTide/vendor/JUCE-8.0.12.tar.gz' in names
    assert archive.read('WolfTide/assets/tide/deep-current-studio/fonts/OFL.txt')
    report['source_archive'] = dict(complete=True, pinned_juce='29396c22c93392d6738e021b83196283d6e4d850',
                                   ibm_plex_sans_ofl=True)

staging.cleanup()
report['limitations'] = ['No Ableton, physical Retina, listening acceptance or interactive Installer.app validation',
                         'Compiled minimum macOS is not a runtime test on macOS 11',
                         'Modern 1.3 saved sounds change; original 1.3.0 is needed for exact restoration']
(dist/'mac-artifacts.json').write_text(json.dumps(report, indent=2)+'\n')
(dist/'Native-Build-ID.txt').write_text(build_id+'\n')
print(json.dumps(report, indent=2))
