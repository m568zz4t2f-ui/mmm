# SPDX-License-Identifier: AGPL-3.0-or-later
"""Portable failure/rollback tests; this does not claim native macOS validation."""
import importlib.util
import hashlib
from pathlib import Path
import json
import plistlib
import shutil
import subprocess
import tempfile
import unittest

spec = importlib.util.spec_from_file_location('mac_install', Path(__file__).resolve().parents[1]/'scripts/mac_install.py')
installer = importlib.util.module_from_spec(spec)
spec.loader.exec_module(installer)


def copy(source, target):
    shutil.copytree(source, target, symlinks=True, copy_function=shutil.copy2)


def bundle(path, version):
    binary = path/'Contents/MacOS/Wolf Tide'
    binary.parent.mkdir(parents=True)
    binary.write_text('binary '+version)
    binary.chmod(0o755)
    (path/'Contents/Resources').mkdir()
    (path/'Contents/Resources/resource').write_text('preserve me')
    (path/'Contents/Resources/link').symlink_to('resource')
    (path/'Contents/Info.plist').write_bytes(plistlib.dumps({'CFBundleIdentifier': 'audio.wolf.tide', 'CFBundleShortVersionString': version, 'CFBundleExecutable': 'Wolf Tide'}))


def validate(path, version):
    data = installer.info(path)
    if data['CFBundleShortVersionString'] != version:
        raise RuntimeError('wrong version')
    if (path/'Contents/MacOS/Wolf Tide').read_text() != 'binary '+version:
        raise RuntimeError('corrupt binary')


class Transactions(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.root = Path(self.temporary.name)
        self.home, self.system, self.art = self.root/'Users/isaiah', self.root/'system', self.root/'artifacts'
        self.user, self.global_ = installer.locations(self.home, self.system)
        self.backups = self.root/'backups'
        for path in self.user: bundle(path, '1.2.0')
        for path in self.global_: bundle(path, '1.2.1')
        for kind, name in zip(installer.FORMATS, installer.BUNDLES):bundle(self.art/kind/name, '1.2.2')
        self.before = {p: installer.digest(p) for p in self.user+self.global_}

    def tearDown(self):
        self.temporary.cleanup()

    def install(self, **overrides):
        options = dict(system=self.system, copy=copy, validate=validate)
        options.update(overrides)
        return installer.install(self.art, self.home, 'system', '1.2.2', self.backups, **options)

    def unchanged(self):
        for path, digest in self.before.items():self.assertEqual(installer.digest(path), digest)

    def test_migrate_and_verified_rollback(self):
        folder = self.install()
        for path in self.global_: validate(path, '1.2.2')
        for path in self.user:self.assertFalse(path.exists())
        self.assertEqual(json.loads((folder/'manifest.json').read_text())['status'], 'complete')
        installer.rollback(folder, copy)
        self.unchanged()
        self.assertEqual(json.loads((folder/'manifest.json').read_text())['status'], 'rolled-back')

    def test_failure_after_replacement_restores_both_locations(self):
        def fail(path, version):
            validate(path, version)
            if path == self.global_[2]:raise RuntimeError('simulated final signature failure')
        with self.assertRaisesRegex(RuntimeError, 'signature'):self.install(validate=fail)
        self.unchanged()

    def test_au_failure_restores_both_locations(self):
        def failure(folder):raise RuntimeError('simulated AU registration failure')
        with self.assertRaisesRegex(RuntimeError, 'AU registration'):self.install(post_install=failure)
        self.unchanged()

    def test_invalid_source_is_rejected_before_mutation(self):
        (self.art/'VST3'/installer.BUNDLES[1]/'Contents/MacOS/Wolf Tide').write_text('truncated')
        with self.assertRaisesRegex(RuntimeError, 'corrupt'):self.install()
        self.unchanged()

    def test_corrupt_backup_is_rejected_before_mutation(self):
        def corrupt(source, target):
            copy(source, target)
            if target.parent.name == 'previous':(target/'Contents/MacOS/Wolf Tide').write_text('broken backup')
        with self.assertRaisesRegex(RuntimeError, 'Backup'):self.install(copy=corrupt)
        self.unchanged()

    def test_user_install_refuses_system_conflicts(self):
        with self.assertRaisesRegex(RuntimeError, 'System Tide copies'):
            installer.install(self.art, self.home, 'user', '1.2.2', self.backups, system=self.system, copy=copy, validate=validate)
        self.unchanged()

    def test_rollback_never_overwrites_later_changes(self):
        folder = self.install()
        (self.global_[0]/'Contents/MacOS/Wolf Tide').write_text('later user update')
        before = installer.digest(self.global_[1])
        with self.assertRaisesRegex(RuntimeError, 'changed since'):installer.rollback(folder, copy)
        self.assertEqual(installer.digest(self.global_[1]), before)

    def test_signing_is_confined_to_verified_staged_copies(self):
        source=self.art/'AU'/installer.BUNDLES[0]
        before=installer.digest(source)
        def sign(stage):(stage/'Contents/Resources/signature').write_text('staged signature')
        self.install(prepare_stage=sign)
        self.assertEqual(installer.digest(source),before)
        self.assertTrue((self.global_[0]/'Contents/Resources/signature').exists())

    def test_symlinked_original_is_rejected(self):
        old = self.user[0]
        saved = old.with_name('original.component')
        old.rename(saved)
        old.symlink_to(saved, target_is_directory=True)
        with self.assertRaisesRegex(RuntimeError, 'symlink'):self.install()
        self.assertTrue(old.is_symlink())


class PackageHooks(unittest.TestCase):
    """Run shell transaction flow with macOS-only tools replaced by test doubles."""
    setUp = Transactions.setUp
    tearDown = Transactions.tearDown
    unchanged = Transactions.unchanged
    def hooks(self):
        folder = self.root/'hooks'
        folder.mkdir()
        buddy = folder/'PlistBuddy'
        buddy.write_text('#!/usr/bin/env python3\nimport plistlib,sys\nwith open(sys.argv[-1],"rb") as f:d=plistlib.load(f)\nprint(d[sys.argv[2].split(":")[-1]])\n')
        buddy.chmod(0o755)
        success = folder/'signature-and-arch-test-double'
        success.write_text('#!/bin/sh\nexit 0\n')
        success.chmod(0o755)
        lipo = folder/'lipo'
        lipo.write_text('#!/usr/bin/env python3\nimport sys\nfrom pathlib import Path\nassert Path(sys.argv[1]).is_file()\nassert sys.argv[2:]==["-verify_arch","arm64","x86_64"]\n')
        lipo.chmod(0o755)
        ditto = folder/'ditto'
        ditto.write_text('#!/usr/bin/env python3\nimport sys,shutil\nshutil.copytree(sys.argv[-2],sys.argv[-1],symlinks=True,copy_function=shutil.copy2)\n')
        ditto.chmod(0o755)
        original = Path(__file__).resolve().parents[1]/'scripts/mac-package-hooks'
        for name in ('preinstall', 'postinstall'):
            script = (original/name).read_text().replace('/Users/', str(self.root/'Users')+'/').replace('/Library/', str(self.system/'Library')+'/').replace('/Applications/', str(self.system/'Applications')+'/')
            script = script.replace('/var/tmp/audio.wolf.tide-install.lock', str(self.root/'package.lock'))
            script = script.replace('/usr/libexec/PlistBuddy', str(buddy)).replace('/usr/bin/ditto', str(ditto))
            script = script.replace('/usr/bin/codesign', str(success)).replace('/usr/bin/lipo', str(lipo))
            (folder/name).write_text(script)
        (folder/'version.txt').write_text('1.2.2\n')
        hashes = [hashlib.sha256((self.art/kind/name/'Contents/MacOS/Wolf Tide').read_bytes()).hexdigest() for kind,name in zip(installer.FORMATS,installer.BUNDLES)]
        (folder/'hashes.txt').write_text('\n'.join(hashes)+'\n')
        return folder

    def run_hook(self, folder, name):
        return subprocess.run(['/bin/sh',str(folder/name),'fake.pkg','/','/'],capture_output=True,text=True)

    def payload(self):
        for target, kind, name in zip(self.global_, installer.FORMATS, installer.BUNDLES):
            shutil.rmtree(target)
            copy(self.art/kind/name,target)

    def test_package_refuses_user_shadows_before_mutation(self):
        folder=self.hooks()
        result=self.run_hook(folder,'preinstall')
        self.assertNotEqual(result.returncode,0)
        self.assertIn('CONFLICT:',result.stderr)
        self.unchanged()
        self.assertFalse((self.root/'package.lock').exists())

    def test_package_verifies_payload_and_keeps_backups(self):
        for path in self.user:shutil.rmtree(path)
        folder=self.hooks()
        result=self.run_hook(folder,'preinstall')
        self.assertEqual(result.returncode,0,result.stderr)
        backup=Path((self.root/'package.lock/backup').read_text().strip())
        self.payload()
        result=self.run_hook(folder,'postinstall')
        self.assertEqual(result.returncode,0,result.stderr)
        self.assertEqual((backup/'status').read_text().strip(),'complete')
        for index,path in enumerate(self.global_):
            self.assertEqual(installer.digest(backup/str(index)/'bundle'),self.before[path])
            validate(path,'1.2.2')
        self.assertFalse((self.root/'package.lock').exists())

    def test_package_failed_payload_rolls_back(self):
        for path in self.user:shutil.rmtree(path)
        folder=self.hooks()
        result=self.run_hook(folder,'preinstall')
        self.assertEqual(result.returncode,0,result.stderr)
        backup=Path((self.root/'package.lock/backup').read_text().strip())
        self.payload()
        (self.global_[1]/'Contents/MacOS/Wolf Tide').write_text('wrong payload digest')
        result=self.run_hook(folder,'postinstall')
        self.assertNotEqual(result.returncode,0)
        self.assertEqual((backup/'status').read_text().strip(),'rolled-back')
        for path in self.global_:self.assertEqual(installer.digest(path),self.before[path])


if __name__ == '__main__':
    unittest.main()
