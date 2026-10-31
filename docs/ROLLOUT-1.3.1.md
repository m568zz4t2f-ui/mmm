# Wolf Tide 1.3.1 — Deep Current Studio Edition test rollout

This candidate combines the completed Deep Current Studio interface at
`4db6ab6d9b2cd52415f4620589647e5302c1963f` with the supplied 1.3.1 processing
sweep based on `f4e4123a6a2c4bba3ed4b29712bce1c7ac55e81c`.
All completed artwork and native interface code are retained. No new generation.

## Read before installing

The modern **Tide 1.3** processing path changes sound. This build does not add a
switch restoring the previous modern algorithms. Existing sessions and presets
still load, but may sound different. Keep the original 1.3.0 installer and project
copies; render critical tracks before replacing your installed plugin. **Legacy
does not reproduce the old modern path.** Restore the original 1.3.0 build for
exact modern sound recall. Plugin identifiers stay the same, so this is an upgrade
of the installed Tide, not a separate side-by-side product.

The only authorized processing-source change is `Source/ModernDSP.h`, pinned to
SHA-256 `e5106dceac1faec3b667840aeb3a5ce5dab762de52484cc8ed0d20c4bfade024`.
The historical freezes are not rewritten. The guard now checks 213 unchanged
processor/parameter/preset files plus this exact upgraded DSP hash. All parameter
IDs/order/ranges, preset files/schema, plugin IDs, frozen legacy DSP, editor
positions, labels and bindings remain unchanged. The historical 64-program
byte-identical result applies to the UI-only 1.3.0 build, not to 1.3.1.

See [the effect-by-effect audit](PROCESSING-SWEEP-1.3.1.md) for all 25 algorithms,
applied fixes, numerical evidence and remaining limitations. The candidate is not
claimed to sound better until musical listening and calibration are complete.

## Build and CI delivery

Run `bash scripts/Build-Mac.command` on macOS with CMake 3.22+, Python 3 and an
installed Apple SDK/compiler. The established build path creates universal arm64
and x86_64 AU, VST3, standalone, an installer and complete corresponding source.
The deployment target is macOS 11.0; CI inspects each Mach-O slice's actual minimum.
This is a compiled minimum, not testing on every supported OS version.

The `Tide 1.3.1 Mac test rollout` Actions workflow runs on native Apple Silicon and
Intel macOS runners, builds the same universal payload, runs CTest, installs the
package on its disposable runner and runs native AU/pluginval validation. It
uploads artifacts and evidence even when later validation fails. A failed job
must be inspected before using any artifact from it. No public release is created.

Files in each successful run's artifact:

- `Wolf-Tide-1.3.1-unsigned.pkg`: system installer.
- `Wolf-Tide-1.3.1-AU.zip`, `-VST3.zip`, `-Standalone.zip`: staged signed bundles.
- `Wolf-Tide-1.3.1-Source.zip`: source, pinned JUCE, all artwork, presets,
  documentation, AGPL and IBM Plex Sans OFL.
- `SHA256SUMS.txt`, `mac-artifacts.json`, native build ID and complete validation logs.

Without Developer ID credentials, bundles are **ad-hoc signed**; the installer is
**unsigned and not notarized**. These are test artifacts. No Apple credentials are
assumed. Do not call them Developer ID signed, Gatekeeper approved or sellable.

## Install / restore

1. Save and close the DAW. Preserve the previous installer and important sessions.
2. Download a completed CI artifact, unzip it, and verify `shasum -a 256 -c
   SHA256SUMS.txt` inside its `dist` directory.
3. Open the `.pkg`. It installs AU/VST3 in `/Library/Audio/Plug-Ins/Components`
   and `/Library/Audio/Plug-Ins/VST3`, the app in `/Applications/Wolf Audio`,
   and source/presets/licenses in `/Library/Application Support/Wolf Audio/Wolf Tide`.
   Installer preflight rejects user shadow copies and preserves previous bundles.
   If macOS blocks the unsigned installer, review it locally; no quarantine or
   security bypass is part of this delivery.
4. Reopen/rescan the host. Verify version **1.3.1** and the artifact's exact build ID.
5. For exact old modern sound, restore the original 1.3.0 installer. The supplied
   `scripts/mac_install.py` supports verified transactional rollback for installs
   made with that tool; package hooks preserve a separate backup journal.

## Acceptance still required

CI is not Ableton or a physical Retina test. Ableton scanning/automation/project
recall, tail/bypass listening, keyboard/browser/dialog review, real-host GUI and
worst-buffer/multi-instance performance, calibrated color/Retina approval and
interactive Installer.app install/uninstall remain pending until separately run.
Command-line installation on a disposable CI Mac does not establish those results.
No automatic uninstall is run on a user's Mac.

Matched-level guitar/keys/vocal/percussion auditions, per-model/preset loudness
calibration, extended decay/stereo/mono/alias/automation tests and an accepted
saved-session migration policy remain release gates. No dynamic gain normalization
or silent preset rewrite is included. At 192 kHz the prepared modern engine alone
uses about 180.28 MB plus legacy/host/UI memory; Mac instance limits need measuring.

Exact commands, exit statuses, runner OS/architecture, source revision, native
build ID, signatures, package payload and checksums are recorded by each CI run.
