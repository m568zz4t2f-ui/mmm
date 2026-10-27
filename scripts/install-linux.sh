#!/bin/sh
set -eu
[ "$(uname -s)" = Linux ] || { echo 'This installer is for Linux x86_64, not macOS.' >&2; exit 1; }
[ "$(uname -m)" = x86_64 ] || { echo 'This build requires x86_64 Linux.' >&2; exit 1; }
TIDE_PACKAGE_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
TIDE_TARGET="${XDG_DATA_HOME:-$HOME/.local/share}/wolf-tide"
mkdir -p "$HOME/.vst3" "$TIDE_TARGET/backups"
if [ -e "$HOME/.vst3/Wolf Tide.vst3" ]; then
 mv "$HOME/.vst3/Wolf Tide.vst3" "$TIDE_TARGET/backups/Wolf Tide-$(date +%Y%m%d-%H%M%S).vst3"
fi
cp -R "$TIDE_PACKAGE_DIR/Wolf Tide.vst3" "$HOME/.vst3/Wolf Tide.vst3"
printf '%s\n' 'Wolf Tide installed in ~/.vst3. Restart your DAW and rescan VST3 plugins.'
