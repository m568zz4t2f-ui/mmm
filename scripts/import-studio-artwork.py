#!/usr/bin/env python3
"""Preserve existing Higgsfield originals and verify the frozen Tide baseline."""
import argparse
import hashlib
import io
import json
from pathlib import Path
import urllib.parse
import urllib.request

from PIL import Image, ImageDraw, ImageOps
from protected_source_hashes import effective_hashes

ROOT = Path(__file__).resolve().parents[1]
DOCS = ROOT / 'docs/deep-current-studio'
RECEIPT = DOCS / 'recovered-artwork-receipt.json'
ALLOWED_HOST = 'd8j0ntlcm91z4.cloudfront.net'
MAX_BYTES = 64 * 1024 * 1024


def digest(data):
    return hashlib.sha256(data).hexdigest()


def protected_sources(allow_visual_changes=False):
    freeze = json.loads((DOCS / 'baseline-freeze.json').read_text())
    files = effective_hashes(freeze['protected_source_and_presets'])
    if not allow_visual_changes:
        files.update(freeze['visual_source_sha256'])
    mismatches = [name for name, sha in files.items()
                  if not (ROOT / name).is_file() or digest((ROOT / name).read_bytes()) != sha]
    if mismatches:
        raise ValueError('Frozen source changed: ' + ', '.join(mismatches))
    return len(files)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--verify-only', action='store_true')
    parser.add_argument('--allow-visual-changes', action='store_true', help='Enforce all protected processing files, allow the six authorized visual/build files to differ.')
    args = parser.parse_args()
    count = protected_sources(args.allow_visual_changes)
    manifest = json.loads((DOCS / 'recovered-higgsfield-jobs.json').read_text())
    assets = manifest['assets']
    assert len(assets) == manifest['completed_originals'] == 29
    assert len({a['job_id'] for a in assets}) == len(assets)
    prior = {}
    if RECEIPT.is_file():
        prior = {a['job_id']: a for a in json.loads(RECEIPT.read_text())['assets']}
    if args.verify_only and len(prior) != len(assets):
        raise ValueError('Import receipt missing or incomplete')
    records, thumbnails = [], []
    for asset in sorted(assets, key=lambda a: a['id']):
        url = urllib.parse.urlsplit(asset['source_url'])
        if url.scheme != 'https' or url.hostname != ALLOWED_HOST or url.username or url.password:
            raise ValueError('Unexpected source URL')
        relative = Path(asset['source_path'])
        if relative.is_absolute() or '..' in relative.parts or relative.parts[:4] != ('assets', 'tide', 'deep-current-studio', 'originals'):
            raise ValueError('Unexpected destination')
        target = ROOT / relative
        if not target.is_file():
            if args.verify_only:
                raise FileNotFoundError(target)
            request = urllib.request.Request(asset['source_url'], headers={'User-Agent': 'WolfTideStudioRecovery/1.0'})
            with urllib.request.urlopen(request, timeout=90) as response:
                if urllib.parse.urlsplit(response.url).hostname != ALLOWED_HOST:
                    raise ValueError('Unexpected download redirect')
                data = response.read(MAX_BYTES + 1)
            if len(data) > MAX_BYTES:
                raise ValueError('Source exceeds 64 MiB')
        else:
            data = target.read_bytes()
        source_hash = digest(data)
        previous = prior.get(asset['job_id'])
        if previous and previous['source_sha256'] != source_hash:
            raise ValueError('Original changed: ' + asset['id'])
        with Image.open(io.BytesIO(data)) as probe:
            if probe.format != 'PNG' or probe.width * probe.height > 40_000_000:
                raise ValueError('Unexpected source image')
            probe.verify()
        with Image.open(io.BytesIO(data)) as source:
            source.load()
            size = list(source.size)
            rgba = source.convert('RGBA')
            alpha = rgba.getchannel('A')
            alpha_bounds = alpha.getbbox()
            alpha_extrema = alpha.getextrema()
            thumb = ImageOps.contain(rgba.convert('RGB'), (380, 250))
        if not target.is_file():
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_bytes(data)
        record = dict(id=asset['id'], job_id=asset['job_id'], source_path=asset['source_path'],
                      source_sha256=source_hash, encoded_bytes=len(data), actual_size=size,
                      reported_size=asset['reported_size'], size_matches_report=size == asset['reported_size'],
                      alpha_bounds=alpha_bounds, alpha_extrema=alpha_extrema,
                      production_approved=False, runtime_exports=[])
        records.append(record)
        thumbnails.append((asset['id'], thumb))
        print(asset['id'], size, source_hash, flush=True)
    if args.verify_only:
        print(f'PASS: {len(records)} originals match receipt; {count} frozen files unchanged')
        return
    receipt = dict(schema_version=1, originals=len(records), frozen_files_verified=count,
                   plugin_code_changed=False, new_generations_submitted=0,
                   note='Integrity checks do not establish runtime readiness, visual approval, or new audio/performance results.',
                   assets=records)
    RECEIPT.write_text(json.dumps(receipt, indent=2) + '\n')
    review = ROOT / 'assets/tide/deep-current-studio/review'
    review.mkdir(parents=True, exist_ok=True)
    for page, start in enumerate(range(0, len(thumbnails), 8), 1):
        sheet = Image.new('RGB', (800, 1160), '#777777')
        draw = ImageDraw.Draw(sheet)
        for slot, (name, thumb) in enumerate(thumbnails[start:start+8]):
            x, y = (slot % 2)*400, (slot // 2)*290
            draw.text((x+8, y+8), name, fill='white')
            sheet.paste(thumb, (x+10+(380-thumb.width)//2, y+30+(250-thumb.height)//2))
        sheet.save(review / f'recovered-{page}.jpg', quality=90)
    print(f'PASS: recovered {len(records)} originals; {count} frozen files unchanged')


if __name__ == '__main__':
    main()
