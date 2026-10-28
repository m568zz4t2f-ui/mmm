#!/usr/bin/env python3
"""Verified Tide installation transaction; native operations are isolated for tests."""
from pathlib import Path
import argparse
import datetime
import hashlib
import json
import os
import plistlib
import re
import shutil
import subprocess
import sys
import tempfile

BUNDLES = ('Wolf Tide.component', 'Wolf Tide.vst3', 'Wolf Tide.app')
FORMATS = ('AU', 'VST3', 'Standalone')


def locations(home, system=Path('/')):
    def under(root):
        return [root/'Library/Audio/Plug-Ins/Components'/BUNDLES[0],
                root/'Library/Audio/Plug-Ins/VST3'/BUNDLES[1],
                root/'Applications/Wolf Audio'/BUNDLES[2]]
    return under(home), under(system)


def info(bundle):
    if bundle.is_symlink():
        raise RuntimeError(f'Refusing a symlinked bundle: {bundle}')
    with (bundle/'Contents/Info.plist').open('rb') as stream:
        data = plistlib.load(stream)
    identity = data.get('CFBundleIdentifier', '')
    if identity != 'audio.wolf.tide' and not identity.startswith('audio.wolf.tide.'):
        raise RuntimeError(f'Unexpected product identity: {bundle}')
    return data


def digest(bundle):
    """Hash complete file content, relative paths, modes and symlink targets."""
    result = hashlib.sha256()
    for path in sorted(bundle.rglob('*')):
        result.update(str(path.relative_to(bundle)).encode()+b'\0')
        result.update(str(path.lstat().st_mode).encode()+b'\0')
        if path.is_symlink():
            result.update(b'link\0'+os.readlink(path).encode()+b'\0')
        elif path.is_file():
            result.update(b'file\0')
            with path.open('rb') as stream:
                for block in iter(lambda: stream.read(1024*1024), b''):
                    result.update(block)
        else:
            result.update(b'dir\0')
    return result.hexdigest()


def native_copy(source, target):
    subprocess.run(['/usr/bin/ditto', '--rsrc', '--extattr', str(source), str(target)], check=True)


def verify_build(bundle, version):
    data = info(bundle)
    if data.get('CFBundleShortVersionString') != version:
        raise RuntimeError(f'Expected {version}, found {data.get("CFBundleShortVersionString")}: {bundle}')
    executable = data.get('CFBundleExecutable', '')
    if not executable or Path(executable).name != executable:
        raise RuntimeError(f'Invalid executable name: {bundle}')
    binary = bundle/'Contents/MacOS'/executable
    architectures = subprocess.check_output(['/usr/bin/lipo', '-archs', str(binary)], text=True).split()
    if not {'arm64', 'x86_64'}.issubset(architectures):
        raise RuntimeError(f'Universal binary required: {bundle}')


def verify(bundle, version):
    verify_build(bundle, version)
    subprocess.run(['/usr/bin/codesign', '--verify', '--strict', str(bundle)], check=True)


def sign_stage(bundle):
    if subprocess.run(['/usr/bin/codesign', '--verify', '--strict', str(bundle)], capture_output=True).returncode:
        subprocess.run(['/usr/bin/codesign', '--force', '--sign', '-', str(bundle)], check=True)


def save_manifest(folder, manifest):
    target = folder/'manifest.json'
    temporary = folder/'manifest.tmp'
    with temporary.open('w') as stream:
        json.dump(manifest, stream, indent=2)
        stream.flush()
        os.fsync(stream.fileno())
    os.replace(temporary, target)


def rollback(folder, copy=native_copy):
    manifest = json.loads((folder/'manifest.json').read_text())
    user, system = locations(Path(manifest['home']), Path(manifest['system']))
    allowed = set(map(str, user+system))
    # Preflight every backup and current destination before replacing anything.
    for index, item in enumerate(manifest['items']):
        target = Path(item['path'])
        if str(target) not in allowed or target.is_symlink():
            raise RuntimeError(f'Unsafe rollback destination: {target}')
        previous = folder/'previous'/str(index)
        if item['existed'] and digest(previous) != item['before']:
            raise RuntimeError(f'Rollback backup failed verification: {previous}')
        if target.exists() and digest(target) not in {item.get('installed'), item.get('before')}:
            raise RuntimeError(f'Destination changed since installation; preserve it before rollback: {target}')
    for index, item in reversed(list(enumerate(manifest['items']))):
        target = Path(item['path'])
        if item['existed']:
            stage = Path(tempfile.mkdtemp(prefix='.tide-restore-', dir=target.parent))
            try:
                restored = stage/target.name
                copy(folder/'previous'/str(index), restored)
                if digest(restored) != item['before']:
                    raise RuntimeError(f'Restored copy failed verification: {target}')
                if target.exists():
                    shutil.rmtree(target)
                os.replace(restored, target)
            finally:
                shutil.rmtree(stage)
        elif target.exists():
            shutil.rmtree(target)
    manifest['status'] = 'rolled-back'
    save_manifest(folder, manifest)


def install(artifacts, home, scope, version, backups, system=Path('/'), copy=native_copy,
            validate=verify, post_install=None, source_validate=None, prepare_stage=None):
    user_targets, system_targets = locations(home, system)
    targets, duplicates = (system_targets, user_targets) if scope == 'system' else (user_targets, system_targets)
    if scope == 'user' and any(path.exists() or path.is_symlink() for path in duplicates):
        raise RuntimeError('System Tide copies exist. Use the system migration (--scope system) to back up both locations and retire user shadows.')
    for path in targets+duplicates:
        if path.exists() or path.is_symlink():
            info(path)  # Identity-check every old bundle before staging or replacing any.
    sources = [artifacts/form/name for form, name in zip(FORMATS, BUNDLES)]
    for source in sources:
        (source_validate or validate)(source, version)
    backups.mkdir(parents=True, exist_ok=True)
    folder = Path(tempfile.mkdtemp(prefix=datetime.datetime.now().strftime('%Y%m%d-%H%M%S-'), dir=backups))
    folder.chmod(0o700)
    (folder/'previous').mkdir()
    manifest = {'version': version, 'scope': scope, 'home': str(home), 'system': str(system),
                'status': 'preparing', 'items': []}
    stages = []
    mutated = False
    try:
        # Stage and validate all three new bundles, preserving original signatures.
        for source, target in zip(sources, targets):
            target.parent.mkdir(parents=True, exist_ok=True)
            stage = Path(tempfile.mkdtemp(prefix='.tide-update-', dir=target.parent))
            stages.append(stage)
            copy(source, stage/target.name)
            if digest(source) != digest(stage/target.name):
                raise RuntimeError(f'Staged copy failed integrity verification: {target}')
            if prepare_stage:
                prepare_stage(stage/target.name)
            validate(stage/target.name, version)
        paths = targets+[path for path in duplicates if path.exists()]
        for index, target in enumerate(paths):
            exists = target.exists()
            item = {'path': str(target), 'existed': exists, 'before': digest(target) if exists else None,
                    'installed': digest(stages[index]/target.name) if index < 3 else None}
            manifest['items'].append(item)
            if exists:
                copy(target, folder/'previous'/str(index))
                if digest(folder/'previous'/str(index)) != item['before']:
                    raise RuntimeError(f'Backup failed integrity verification: {target}')
        manifest['status'] = 'backed-up'
        save_manifest(folder, manifest)
        for index, item in enumerate(manifest['items']):
            target = Path(item['path'])
            if target.exists() and digest(target) != item['before']:
                raise RuntimeError(f'Installation changed during preflight: {target}')
            mutated = True
            if target.exists():
                shutil.rmtree(target)  # Every original now has a verified, durable backup.
            if index < 3:
                os.replace(stages[index]/target.name, target)
                validate(target, version)
                if digest(target) != item['installed']:
                    raise RuntimeError(f'Installed copy failed integrity verification: {target}')
            manifest['status'] = 'installing'
            save_manifest(folder, manifest)
        if post_install:
            post_install(folder)
        manifest['status'] = 'complete'
        save_manifest(folder, manifest)
        return folder
    except BaseException:
        if mutated:
            rollback(folder, copy)
        raise
    finally:
        for stage in stages:
            shutil.rmtree(stage, ignore_errors=True)


def refresh_and_validate_au(home, version, folder):
    import pwd
    user = pwd.getpwuid(home.stat().st_uid).pw_name
    prefix = ['/usr/bin/sudo', '-u', user] if os.geteuid() == 0 else []
    subprocess.run(prefix+['/usr/bin/killall', 'AudioComponentRegistrar'], capture_output=True)
    result = subprocess.run(prefix+['/usr/bin/auval', '-v', 'aufx', 'Wtid', 'Wolf'], capture_output=True, text=True)
    (folder/'auval.log').write_text(result.stdout+result.stderr)
    if result.returncode or not re.search(r'Component Version:\s*'+re.escape(version)+r'\b', result.stdout):
        raise RuntimeError(f'AU validation did not confirm {version}; see {folder}/auval.log')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--scope', choices=('user', 'system'), default='user')
    parser.add_argument('--user-home', type=Path, default=Path.home())
    parser.add_argument('--build-dir', type=Path, default=Path(os.environ.get('TIDE_BUILD_DIR', 'build-mac')))
    parser.add_argument('--check', action='store_true', help='Report both installation locations without modifying them')
    parser.add_argument('--rollback', type=Path, help='Restore a verified installation transaction')
    args = parser.parse_args()
    if sys.platform != 'darwin':
        parser.error('Native installation requires macOS; portable transaction tests run separately.')
    root = Path(__file__).resolve().parent.parent
    if args.check:
        for group in locations(args.user_home):
            for path in group:
                print(path, info(path).get('CFBundleShortVersionString') if path.exists() else 'not installed')
        return
    if args.scope == 'system' and (os.geteuid() != 0 or args.user_home == Path('/var/root')):
        parser.error('System migration requires sudo and an explicit --user-home for the user whose shadow copies are retired.')
    if args.rollback:
        rollback(args.rollback)
        print('Rollback verified:', args.rollback)
        return
    version = re.search(r'project\(WolfTide VERSION ([0-9.]+)', (root/'CMakeLists.txt').read_text()).group(1)
    artifacts = (root/args.build_dir/'WolfTide_artefacts/Release').resolve()
    backups = (Path('/') if args.scope == 'system' else args.user_home)/'Library/Application Support/Wolf Audio/Wolf Tide/install-backups'
    folder = install(artifacts, args.user_home, args.scope, version, backups,
                     post_install=lambda folder: refresh_and_validate_au(args.user_home, version, folder),
                     source_validate=verify_build, prepare_stage=sign_stage)
    print('Installed and AU-validated', version, '\nVerified rollback:', folder)
    print('Reopen/rescan your DAW when convenient. No DAW session was closed or modified.')


if __name__ == '__main__':
    main()
