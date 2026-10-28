# Build the Wolf Tide 1.3.0 Mac installer

This source delivery is a build/test candidate. It contains the integrated plugin, all 25 processing paths, four-slot UI/state/presets, pinned JUCE source, automated tests, migration/rollback tools and package scripts. It contains no macOS binaries. Native Mac build, AU validation, installed-package checks and human listening acceptance must be performed on the Mac before distribution.

## Short path

1. Download and unzip the **Source ZIP** into a new folder. Keep the original 1.2.x source/installers.
2. Open Terminal in the extracted `WolfTide` folder. Install Xcode Command Line Tools (`xcode-select --install` if absent), Python 3, and CMake 3.22 or newer. Homebrew users can run `brew install cmake python` if needed. The full Xcode app is not required for the default Makefiles build, but a current macOS SDK is.
3. Run:

```sh
bash scripts/Build-Mac.command
```

You can also open that command file from Finder. Terminal gives clearer diagnostics if Gatekeeper or file permissions prevent launching it. The build does **not** require downloading JUCE: the ZIP carries the pinned 8.0.12 source. Allow several minutes and sufficient RAM for compilation. Tests that initialize native UI should run in a logged-in desktop session.

The script builds Release **arm64 + x86_64**, runs all CTest suites, stages AU/VST3/standalone, checks both architecture slices and bundle versions, signs/verifies staged copies, and creates the package. It stops on errors and writes `dist/Mac-build-<timestamp>.log`.

Output without Developer ID credentials:

```
dist/Wolf-Tide-1.3.0-unsigned.pkg
```

This is for local testing, with ad-hoc signed bundles. It is not Developer ID signed or notarized. Do not present it as a notarized public installer. Native binaries remain in:

```
build-mac/WolfTide_artefacts/Release/AU/Wolf Tide.component
build-mac/WolfTide_artefacts/Release/VST3/Wolf Tide.vst3
build-mac/WolfTide_artefacts/Release/Standalone/Wolf Tide.app
```

## Your existing duplicate installs

The earlier report found system 1.2.1 and user 1.2.0 copies. That report has not been verified or changed by this cloud build. The package preflight refuses shadow copies so the DAW cannot silently load the old user plugin. Close the DAW yourself after saving work; the scripts do not close sessions.

Inspect and migrate the selected user's installations after building:

```sh
python3 scripts/install-user-mac.py --user-home /Users/isaiahwersky --check
sudo python3 scripts/install-user-mac.py --scope system --user-home /Users/isaiahwersky
```

Migration validates the staged 1.3.0 binaries, makes verified rollback copies of system/user bundles, installs the canonical system copies, retires the selected user's shadows, refreshes AU registration and checks **auval's actual Component Version**. It rolls back on failure. It prints its transaction folder and writes `manifest.json` and `auval.log`. Other users' shadow copies must be resolved separately before the system package can install.

To restore that transaction:

```sh
sudo python3 scripts/install-user-mac.py --scope system --user-home /Users/isaiahwersky --rollback '/Library/Application Support/Wolf Audio/Wolf Tide/install-backups/<transaction>'
```

Refresh and revalidate the restored AU in the logged-in user's session afterward.

## Native validation and release evidence

```sh
ctest --test-dir build-mac -C Release --output-on-failure
auval -v aufx Wtid Wolf
```

Confirm `Component Version: 1.3.0`. Run pluginval strictness 5 or higher on the **new Mac VST3**, save its log, and inspect both architectures on matching hardware (Rosetta testing alone does not establish native Apple Silicon behavior). Open the standalone and your DAW, check each advanced page at your display scale, reopen an old project in Legacy, and save/reopen a four-slot 1.3 preset. Test the actual Installer.app package and the documented rollback on a saved/isolated setup. The portable installer tests are not an Installer.app acceptance test.

Listen to guitar, voice, piano/keys, bass, percussion, transients and sustained/chord material at 44.1/48/96/192 kHz. Check mono fold-down, loud/quiet sources, tempo changes, delay-time sweeps, holds, quick model browsing, bypass/spillover and extreme feedback. Adjust Amount and Output to the source. Numerical/recording energy measurements in the ZIP do not replace these listening checks.

`tests/modern_audio.cpp` accepts local WAV/OGG recordings through the optional `tide_modern_audio` target; it reports raw and calibrated return energy/peak/correlation without copying your audio. `tide_modern_profile` prints per-machine and four-slot CPU measurements. No internet audio or your recordings are included.

## Developer ID distribution

With your own certificates and an existing notarytool Keychain profile configured on the Mac:

```sh
TIDE_APP_IDENTITY='Developer ID Application: YOUR NAME (TEAMID)' \
TIDE_INSTALLER_IDENTITY='Developer ID Installer: YOUR NAME (TEAMID)' \
TIDE_NOTARY_PROFILE='YOUR_EXISTING_KEYCHAIN_PROFILE' \
bash scripts/Build-Mac.command
```

No credentials are supplied by the source package. The script produces `dist/Wolf-Tide-1.3.0.pkg`, verifies its signature, submits it for notarization, staples/verifies the ticket and runs Gatekeeper assessment. Without a notary profile, it explicitly reports that the signed package is not notarized.

The package contains corresponding source, the license, algorithm guide and factory presets. Keep the build log, CTest results, AU/pluginval results, installation/rollback manifest and final SHA-256 alongside the package:

```sh
shasum -a 256 dist/Wolf-Tide-1.3.0*.pkg
```

## Troubleshooting

- CMake missing: install CMake, then reopen Terminal so its executable is on PATH.
- Compiler/SDK missing: complete `xcode-select --install`, then check `xcrun --find clang++`.
- Wrong source/version: extract into a clean directory; the build checks 1.3.0 in all payload bundles. Packaging always runs an incremental build and tests to avoid stale binaries.
- `auval` finds 1.2.0/1.2.1: inspect both user/system locations and run the verified migration; do not merely rename plugin files.
- Universal build fails: inspect the first compiler/linker error in the log. Do not claim a universal installer from an Intel-only successful build.
- Package conflict: preflight reports the exact shadow-copy paths. Preserve their backups before migration.
- No Mac build access from this chat: run the commands above locally; the ZIP is sufficient to build without the cloud workspace.
