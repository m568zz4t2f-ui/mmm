#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")/.."
if [ "$(uname -s)" != Darwin ]; then echo 'Open this build command on your Mac.' >&2; exit 1; fi
for tool in cmake python3 xcrun; do
 command -v "$tool" >/dev/null || { echo "Missing $tool. See docs/BUILD-MAC-1.3.md." >&2; exit 1; }
done
xcrun --find clang++ >/dev/null
mkdir -p dist
TIDE_BUILD_LOG="dist/Mac-build-$(date +%Y%m%d-%H%M%S).log"
{
 echo 'Wolf Tide 1.3.1 Deep Current Studio Edition — universal test build'
 sw_vers
 cmake --version
 xcrun clang++ --version
 bash scripts/package-mac.sh
 echo 'Build complete. See dist for the package and this log.'
 echo 'Run native AU validation and your DAW listening checks before distributing.'
} 2>&1 | tee "$TIDE_BUILD_LOG"
