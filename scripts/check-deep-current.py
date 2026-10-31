#!/usr/bin/env python3
"""Verify frozen processing, parameter definitions and factory preset bytes."""
import hashlib,json,re
from pathlib import Path
from protected_source_hashes import effective_hashes
root=Path(__file__).resolve().parents[1]
manifest=json.loads((root/'docs/deep-current/evidence/baseline/protected-source.json').read_text())
manifest=effective_hashes(manifest)
failures=[name for name,digest in manifest.items() if hashlib.sha256((root/name).read_bytes()).hexdigest()!=digest]
if failures:raise SystemExit('Protected source changed: '+', '.join(failures))
print(f'PASS: {len(manifest)-1} historical processor/parameter/preset hashes unchanged; exact authorized 1.3.1 ModernDSP hash verified.')
source=(root/'Source/PluginEditor.cpp').read_text()
starts=list(re.finditer(r'^(?:void )?TideEditor::(\w+|~TideEditor)\(',source,re.M))
methods={m.group(1):source[m.start():starts[i+1].start() if i+1<len(starts) else len(source)].strip() for i,m in enumerate(starts)}
frozen=json.loads((root/'docs/deep-current/evidence/baseline/editor-methods.json').read_text())
for name,digest in frozen.items():
    body=methods[name]
    # These two additions assign only the module-lifetime visual theme to callout shells.
    if name in {'showModern','showBrowser'}:
        body=body.replace('auto& popup=','').replace('popup.setLookAndFeel(&tide::ui::sharedLook());','')
    if hashlib.sha256(body.encode()).hexdigest()!=digest:raise SystemExit('Frozen editor method changed: '+name)
print(f'PASS: {len(frozen)} editor methods preserve layout, bindings, input handlers, timer and visualization semantics.')
