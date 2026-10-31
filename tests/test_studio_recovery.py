"""Check original integrity and the frozen-source guard without network access."""
import contextlib
import hashlib
import importlib.util
import io
import json
from pathlib import Path
import sys
import tempfile
import unittest
from PIL import Image

SCRIPT = Path(__file__).resolve().parents[1] / 'scripts/import-studio-artwork.py'
sys.path.insert(0, str(SCRIPT.parent))
spec = importlib.util.spec_from_file_location('studio_import', SCRIPT)
recovery = importlib.util.module_from_spec(spec)
spec.loader.exec_module(recovery)


class RecoveryTests(unittest.TestCase):
    def test_processing_upgrade_is_an_exact_hash_replacement(self):
        from protected_source_hashes import effective_hashes, MODERN_BASE, MODERN_131
        frozen = {'Source/ModernDSP.h': MODERN_BASE, 'Source/TideDSP.h': 'legacy'}
        expected = effective_hashes(frozen)
        self.assertEqual(expected, {'Source/ModernDSP.h': MODERN_131, 'Source/TideDSP.h': 'legacy'})
        self.assertEqual(frozen['Source/ModernDSP.h'], MODERN_BASE)
        with self.assertRaisesRegex(ValueError, 'historical ModernDSP baseline'):
            effective_hashes({'Source/ModernDSP.h': 'unrecognized source'})

    def test_integrity_receipt_and_source_guard(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            recovery.ROOT = root
            recovery.DOCS = root / 'docs/deep-current-studio'
            recovery.DOCS.mkdir(parents=True)
            recovery.RECEIPT = recovery.DOCS / 'recovered-artwork-receipt.json'
            protected = root / 'Source.cpp'
            protected.write_bytes(b'frozen source')
            freeze = {'protected_source_and_presets': {'Source.cpp': hashlib.sha256(protected.read_bytes()).hexdigest()}, 'visual_source_sha256': {}}
            (recovery.DOCS / 'baseline-freeze.json').write_text(json.dumps(freeze))
            assets = []
            for index in range(29):
                relative = f'assets/tide/deep-current-studio/originals/{index}.png'
                target = root / relative
                target.parent.mkdir(parents=True, exist_ok=True)
                Image.new('RGB', (8, 6), '#246e76').save(target)
                assets.append(dict(id=str(index), job_id=str(index), source_path=relative,
                                   source_url=f'https://{recovery.ALLOWED_HOST}/{index}.png', reported_size=[10, 10]))
            (recovery.DOCS / 'recovered-higgsfield-jobs.json').write_text(json.dumps({'completed_originals':29, 'assets':assets}))
            previous = sys.argv
            try:
                with contextlib.redirect_stdout(io.StringIO()):
                    sys.argv = [str(SCRIPT)]
                    recovery.main()
                    receipt = json.loads(recovery.RECEIPT.read_text())
                    self.assertEqual(receipt['assets'][0]['actual_size'], [8, 6])
                    self.assertFalse(receipt['assets'][0]['size_matches_report'])
                    sys.argv = [str(SCRIPT), '--verify-only']
                    recovery.main()
                    original = root / assets[0]['source_path']
                    Image.new('RGB', (8, 6), 'white').save(original)
                    with self.assertRaisesRegex(ValueError, 'Original changed'):
                        recovery.main()
                    protected.write_bytes(b'modified source')
                    with self.assertRaisesRegex(ValueError, 'Frozen source changed'):
                        recovery.main()
            finally:
                sys.argv = previous

    def test_visual_permission_never_exempts_processing(self):
        with tempfile.TemporaryDirectory() as folder:
            root=Path(folder);recovery.ROOT=root;recovery.DOCS=root/'docs';recovery.DOCS.mkdir()
            (root/'processor.cpp').write_bytes(b'audio');(root/'editor.cpp').write_bytes(b'visual')
            freeze={'protected_source_and_presets':{'processor.cpp':hashlib.sha256(b'audio').hexdigest()},'visual_source_sha256':{'editor.cpp':hashlib.sha256(b'visual').hexdigest()}}
            (recovery.DOCS/'baseline-freeze.json').write_text(json.dumps(freeze))
            (root/'editor.cpp').write_bytes(b'authorized visual change')
            with self.assertRaisesRegex(ValueError,'Frozen source changed'):recovery.protected_sources()
            self.assertEqual(recovery.protected_sources(allow_visual_changes=True),1)
            (root/'processor.cpp').write_bytes(b'unauthorized audio change')
            with self.assertRaisesRegex(ValueError,'Frozen source changed'):recovery.protected_sources(allow_visual_changes=True)


if __name__ == '__main__':
    unittest.main()
