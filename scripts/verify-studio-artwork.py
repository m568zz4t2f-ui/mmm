#!/usr/bin/env python3
"""Verify immutable originals/processing and prepared resources; report authorized UI edits."""
import hashlib,json,struct
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
DOCS=ROOT/'docs/deep-current-studio'
def digest(path):return hashlib.sha256((ROOT/path).read_bytes()).hexdigest()
def verify():
 freeze=json.loads((DOCS/'baseline-freeze.json').read_text())
 for path,sha in freeze['protected_source_and_presets'].items():
  if digest(path)!=sha:raise ValueError('Protected source changed: '+path)
 receipt=json.loads((DOCS/'recovered-artwork-receipt.json').read_text())
 for a in receipt['assets']:
  if digest(a['source_path'])!=a['source_sha256']:raise ValueError('Original changed: '+a['source_path'])
 runtime=json.loads((ROOT/'assets/tide/deep-current-studio/runtime/manifest.json').read_text())
 known={a['source_path']:a['source_sha256'] for a in receipt['assets']}
 for e in runtime['exports']:
  if known.get(e['source_path'])!=e['source_sha256']:raise ValueError('Unrecovered artwork: '+e['name'])
  data=(ROOT/e['export_path']).read_bytes()
  if hashlib.sha256(data).hexdigest()!=e['export_sha256']:raise ValueError('Export changed: '+e['name'])
  if data[:8]!=b'\x89PNG\r\n\x1a\n' or list(struct.unpack('>II',data[16:24]))!=e['export_dimensions']:raise ValueError('Invalid export dimensions: '+e['name'])
  if e['slice_margins']:
   left,top,right,bottom=e['slice_margins'];w,h=e['export_dimensions']
   if min(left,top,right,bottom)<0 or left+right>=w or top+bottom>=h:raise ValueError('Invalid slices: '+e['name'])
 decoded=sum(e['decoded_bytes'] for e in runtime['exports'])
 if decoded>32*1024*1024:raise ValueError('Immutable artwork exceeds 32 MiB target')
 changes={p:{'baseline_sha256':sha,'current_sha256':digest(p),'changed':digest(p)!=sha} for p,sha in freeze['visual_source_sha256'].items()}
 return dict(protected_files_verified=len(freeze['protected_source_and_presets']),originals_verified=len(receipt['assets']),runtime_exports_verified=len(runtime['exports']),immutable_artwork_bytes=decoded,authorized_visual_build_files=changes)
if __name__=='__main__':
 report=verify();print(json.dumps(report,indent=2))
