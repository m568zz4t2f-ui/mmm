#!/bin/bash
# Build-machine script. Recipients only open the resulting .pkg installer.
set -euo pipefail
cd "$(dirname "$0")/.."
[ "$(uname -s)" = Darwin ] || { echo 'A macOS build host is required.' >&2; exit 1; }
TIDE_BUILD_DIR="${TIDE_BUILD_DIR:-build-mac}"
# Always bring the binaries up to date before packaging corresponding source.
# Incremental builds are cheap; a stale same-version payload is not acceptable.
TIDE_VERSION=$(python3 - <<'PYVERSION'
import re
from pathlib import Path
print(re.search(r'project\(WolfTide VERSION ([0-9.]+)',Path('CMakeLists.txt').read_text()).group(1))
PYVERSION
)
cmake -S . -B "$TIDE_BUILD_DIR" -DCMAKE_BUILD_TYPE=Release '-DCMAKE_OSX_ARCHITECTURES=arm64;x86_64' -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0
cmake --build "$TIDE_BUILD_DIR" --config Release --parallel 2
ctest --test-dir "$TIDE_BUILD_DIR" -C Release --output-on-failure
TIDE_ART="$TIDE_BUILD_DIR/WolfTide_artefacts/Release"
TIDE_STAGE="$(mktemp -d "${TMPDIR:-/tmp}/wolf-tide-package.XXXXXX")"
trap 'rm -rf "$TIDE_STAGE"' EXIT
mkdir -p "$TIDE_STAGE/root/Library/Audio/Plug-Ins/Components" "$TIDE_STAGE/root/Library/Audio/Plug-Ins/VST3" "$TIDE_STAGE/root/Applications/Wolf Audio" "$TIDE_STAGE/root/Library/Application Support/Wolf Audio/Wolf Tide"
ditto "$TIDE_ART/AU/Wolf Tide.component" "$TIDE_STAGE/root/Library/Audio/Plug-Ins/Components/Wolf Tide.component"
ditto "$TIDE_ART/VST3/Wolf Tide.vst3" "$TIDE_STAGE/root/Library/Audio/Plug-Ins/VST3/Wolf Tide.vst3"
ditto "$TIDE_ART/Standalone/Wolf Tide.app" "$TIDE_STAGE/root/Applications/Wolf Audio/Wolf Tide.app"
for TIDE_BUNDLE in "$TIDE_STAGE/root/Library/Audio/Plug-Ins/Components/Wolf Tide.component" "$TIDE_STAGE/root/Library/Audio/Plug-Ins/VST3/Wolf Tide.vst3" "$TIDE_STAGE/root/Applications/Wolf Audio/Wolf Tide.app"; do
 if [ -n "${TIDE_APP_IDENTITY:-}" ]; then
  codesign --force --options runtime --timestamp --sign "$TIDE_APP_IDENTITY" "$TIDE_BUNDLE"
 else
  codesign --force --sign - "$TIDE_BUNDLE"
 fi
 codesign --verify --strict "$TIDE_BUNDLE"
done
# Complete corresponding source travels with the installer, including pinned JUCE.
python3 scripts/source-archive.py "$TIDE_STAGE/root/Library/Application Support/Wolf Audio/Wolf Tide/Source.zip"
cp docs/QUICK-START.md "$TIDE_STAGE/root/Library/Application Support/Wolf Audio/Wolf Tide/Quick Start.md"
cp LICENSE "$TIDE_STAGE/root/Library/Application Support/Wolf Audio/Wolf Tide/LICENSE.txt"
cp assets/tide/deep-current-studio/fonts/OFL.txt "$TIDE_STAGE/root/Library/Application Support/Wolf Audio/Wolf Tide/IBM-Plex-Sans-OFL.txt"
cp docs/ALGORITHMS-1.3.md "$TIDE_STAGE/root/Library/Application Support/Wolf Audio/Wolf Tide/Algorithms.md"
cp docs/ROLLOUT-1.3.1.md "$TIDE_STAGE/root/Library/Application Support/Wolf Audio/Wolf Tide/Read Before Installing.md"
if [ -d Factory-Presets ]; then
 ditto Factory-Presets "$TIDE_STAGE/root/Library/Application Support/Wolf Audio/Wolf Tide/Factory Presets"
fi
# Version and executable digests come from the signed payload, never a display label.
mkdir -p "$TIDE_STAGE/hooks"
cp scripts/mac-package-hooks/preinstall scripts/mac-package-hooks/postinstall "$TIDE_STAGE/hooks/"
chmod 755 "$TIDE_STAGE/hooks/preinstall" "$TIDE_STAGE/hooks/postinstall"
printf '%s\n' "$TIDE_VERSION" > "$TIDE_STAGE/hooks/version.txt"
python3 - "$TIDE_STAGE/root" "$TIDE_STAGE/hooks/hashes.txt" "$TIDE_VERSION" <<'PYHASH'
import hashlib,plistlib,subprocess,sys
from pathlib import Path
root=Path(sys.argv[1]);hashes=[]
for relative in ['Library/Audio/Plug-Ins/Components/Wolf Tide.component','Library/Audio/Plug-Ins/VST3/Wolf Tide.vst3','Applications/Wolf Audio/Wolf Tide.app']:
 bundle=root/relative
 with (bundle/'Contents/Info.plist').open('rb') as stream: info=plistlib.load(stream)
 if info['CFBundleShortVersionString']!=sys.argv[3]:raise SystemExit('Payload version mismatch: '+str(bundle))
 executable=info['CFBundleExecutable']
 if Path(executable).name!=executable:raise SystemExit('Invalid payload executable')
 binary=bundle/'Contents/MacOS'/executable
 subprocess.run(['lipo','-verify_arch','arm64','x86_64',str(binary)],check=True)
 hashes.append(hashlib.sha256(binary.read_bytes()).hexdigest())
Path(sys.argv[2]).write_text('\n'.join(hashes)+'\n')
PYHASH
mkdir -p dist
TIDE_UNSIGNED="dist/Wolf-Tide-$TIDE_VERSION-unsigned.pkg"
pkgbuild --analyze --root "$TIDE_STAGE/root" "$TIDE_STAGE/components.plist"
python3 - "$TIDE_STAGE/components.plist" <<'PYCOMP'
import plistlib,sys
from pathlib import Path
path=Path(sys.argv[1]);components=plistlib.loads(path.read_bytes())
for component in components:
 component['BundleIsRelocatable']=False
 component['BundleIsVersionChecked']=True
 component['BundleOverwriteAction']='upgrade'
path.write_bytes(plistlib.dumps(components))
PYCOMP
pkgbuild --scripts "$TIDE_STAGE/hooks" --component-plist "$TIDE_STAGE/components.plist" --root "$TIDE_STAGE/root" --identifier audio.wolf.tide.pkg --version "$TIDE_VERSION" --install-location / "$TIDE_UNSIGNED"
pkgutil --payload-files "$TIDE_UNSIGNED" > dist/mac-payload.txt
if [ -n "${TIDE_INSTALLER_IDENTITY:-}" ]; then
 [ -n "${TIDE_APP_IDENTITY:-}" ] || { echo 'Developer ID Application identity required for signed release.' >&2; exit 1; }
 TIDE_FINAL="dist/Wolf-Tide-$TIDE_VERSION.pkg"
 productsign --sign "$TIDE_INSTALLER_IDENTITY" "$TIDE_UNSIGNED" "$TIDE_FINAL"
 pkgutil --check-signature "$TIDE_FINAL"
 if [ -n "${TIDE_NOTARY_PROFILE:-}" ]; then
  xcrun notarytool submit "$TIDE_FINAL" --keychain-profile "$TIDE_NOTARY_PROFILE" --wait
  xcrun stapler staple "$TIDE_FINAL"
  xcrun stapler validate "$TIDE_FINAL"
  spctl --assess --type install --verbose=2 "$TIDE_FINAL"
 else
  echo 'Signed but NOT notarized. Set TIDE_NOTARY_PROFILE for a distribution release.'
 fi
else
 echo "Created $TIDE_UNSIGNED (ad-hoc plugin signatures; installer NOT Developer ID signed or notarized)."
fi
# Deliver the verified staged bundles as well as the installer; preserve signatures.
cp "$TIDE_STAGE/root/Library/Application Support/Wolf Audio/Wolf Tide/Source.zip" "dist/Wolf-Tide-$TIDE_VERSION-Source.zip"
ditto -c -k --sequesterRsrc --keepParent "$TIDE_STAGE/root/Library/Audio/Plug-Ins/Components/Wolf Tide.component" "dist/Wolf-Tide-$TIDE_VERSION-AU.zip"
ditto -c -k --sequesterRsrc --keepParent "$TIDE_STAGE/root/Library/Audio/Plug-Ins/VST3/Wolf Tide.vst3" "dist/Wolf-Tide-$TIDE_VERSION-VST3.zip"
ditto -c -k --sequesterRsrc --keepParent "$TIDE_STAGE/root/Applications/Wolf Audio/Wolf Tide.app" "dist/Wolf-Tide-$TIDE_VERSION-Standalone.zip"
cp docs/ROLLOUT-1.3.1.md dist/Read-Before-Installing.md
