#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")/.."
PRISM_ART=build-mac/WolfPrism_artefacts/Release
PRISM_STAGE="$(mktemp -d "${TMPDIR:-/tmp}/wolf-prism.XXXXXX")"
trap 'rm -rf "$PRISM_STAGE"' EXIT
mkdir -p "$PRISM_STAGE/root/Library/Audio/Plug-Ins/Components" "$PRISM_STAGE/root/Library/Audio/Plug-Ins/VST3" "$PRISM_STAGE/root/Applications/Wolf Audio" "$PRISM_STAGE/root/Library/Application Support/Wolf Audio/Wolf Prism" "$PRISM_STAGE/hooks" dist
for PRISM_FORMAT in AU VST3 Standalone; do
 case "$PRISM_FORMAT" in
 AU) PRISM_NAME='Wolf Prism.component'; PRISM_DEST='Library/Audio/Plug-Ins/Components';;
 VST3) PRISM_NAME='Wolf Prism.vst3'; PRISM_DEST='Library/Audio/Plug-Ins/VST3';;
 Standalone) PRISM_NAME='Wolf Prism.app'; PRISM_DEST='Applications/Wolf Audio';;
 esac
 ditto "$PRISM_ART/$PRISM_FORMAT/$PRISM_NAME" "$PRISM_STAGE/root/$PRISM_DEST/$PRISM_NAME"
 codesign --force --sign - "$PRISM_STAGE/root/$PRISM_DEST/$PRISM_NAME"
 codesign --verify --strict "$PRISM_STAGE/root/$PRISM_DEST/$PRISM_NAME"
done
python3 scripts/source-archive.py "$PRISM_STAGE/root/Library/Application Support/Wolf Audio/Wolf Prism/Source.zip"
cp README.md "$PRISM_STAGE/root/Library/Application Support/Wolf Audio/Wolf Prism/Read Me.md"
cp LICENSE "$PRISM_STAGE/root/Library/Application Support/Wolf Audio/Wolf Prism/LICENSE.txt"
cat > "$PRISM_STAGE/hooks/preinstall" <<'HOOK'
#!/bin/bash
set -euo pipefail
prism_target="${3:-/}"
prism_backup="$prism_target/Library/Application Support/Wolf Audio/Wolf Prism/Backups/$(date +%Y%m%d-%H%M%S)"
for path in 'Library/Audio/Plug-Ins/Components/Wolf Prism.component' 'Library/Audio/Plug-Ins/VST3/Wolf Prism.vst3' 'Applications/Wolf Audio/Wolf Prism.app'; do
 if [ -e "$prism_target/$path" ]; then
  mkdir -p "$prism_backup/$(dirname "$path")"
  ditto "$prism_target/$path" "$prism_backup/$path"
 fi
done
HOOK
chmod +x "$PRISM_STAGE/hooks/preinstall"
pkgbuild --analyze --root "$PRISM_STAGE/root" "$PRISM_STAGE/components.plist"
python3 - "$PRISM_STAGE/components.plist" <<'PY'
import sys,plistlib
from pathlib import Path
p=Path(sys.argv[1]);items=plistlib.loads(p.read_bytes())
for i in items:i.update(BundleIsRelocatable=False,BundleIsVersionChecked=True,BundleOverwriteAction='upgrade')
p.write_bytes(plistlib.dumps(items))
PY
pkgbuild --component-plist "$PRISM_STAGE/components.plist" --root "$PRISM_STAGE/root" --scripts "$PRISM_STAGE/hooks" --identifier audio.wolf.prism.pkg --version 0.1.0 --install-location / dist/Wolf-Prism-0.1.0-unsigned.pkg
pkgutil --payload-files dist/Wolf-Prism-0.1.0-unsigned.pkg > dist/mac-payload.txt
python3 scripts/source-archive.py dist/Wolf-Prism-0.1.0-Source.zip
shasum -a 256 dist/Wolf-Prism-0.1.0-unsigned.pkg dist/Wolf-Prism-0.1.0-Source.zip > dist/SHA256SUMS.txt
