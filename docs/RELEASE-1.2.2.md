# Wolf Tide 1.2.2 — preset and installation reliability

This maintenance update preserves the 1.2.1 DSP. The reported Mac installation has system 1.2.1 bundles and older user copies; this session cannot access that Mac and has not changed those live installations. Native Mac release gates remain pending.

## Preset identity and UI

Delay, reverb and combined identities are stored independently, including their saved parameter snapshots and Factory/User origin. Editing, host automation, partial recalls and locks are compared against the saved settings rather than the last menu click. Returning exactly to the saved settings clears Modified. External portable files are treated as User sounds; built-in entries are Factory sounds.

One `refreshSelectors()` function updates active model selectors, selected menu items and displayed names. Dropdowns, successful browser save/load, file import/export, Undo Load and A/B all use it. The existing visible-editor timer refreshes host automation and project recall (12–24 Hz); hidden editors suspend the timer and refresh when reopened. Preset metadata is not updated from the sample-processing callback.

Combined recall updates unlocked sides and preserves locked side identities. Loading or saving another scope does not erase a named user sound on the opposite side. Independent identities and modified baselines persist in schema-3 session state. States without the new metadata still load; factory identity is reconstructed from the saved program, and missing user side names are left custom rather than invented. The old 1.2.1 format never saved independent side names, so names absent from it cannot be recovered.

The prominent `Wolf Tide v1.2.2 / <source build ID>` label uses the JUCE compiled release version and a CMake-generated source hash. Legacy/Modern 1.2 remains an unchanged sound-generation choice. It is not a release label. The build label is an identifier, not a replacement for executable checksum/signature verification.

## Installation and rollback

The native migration tool is `scripts/mac_install.py`; `install-user-mac.py` remains a compatibility entry point. It:

- Reports both user/system AU, VST3 and standalone locations with `--check`.
- Refuses an ordinary user install when system copies already exist.
- Supports an explicit privileged system migration for the named user's home, with all three new bundles staged and checked before replacement.
- Verifies product identity, compiled version, universal architecture slices and signatures. Unsigned developer builds are signed only in verified staged copies; existing valid signatures are retained.
- Copies and verifies the complete original file trees (including modes and symlink targets) from both locations before changing either. Native copies use `ditto --rsrc --extattr` to preserve Mac metadata.
- Installs one canonical system set and retires that user's shadow copies. A persisted manifest records before/after digests. Failures, including AU registration/version mismatch, trigger verified rollback of both locations.
- Refreshes the target user's AudioComponentRegistrar and runs `auval -v aufx Wtid Wolf`; success must explicitly identify the requested release. The output is retained with the backup.
- Supports `--rollback <transaction folder>`. Rollback refuses changed destinations or corrupted backups instead of overwriting a later installation.

Other users' homes are not modified by the migration command. The system package preflight scans all `/Users/*` homes for shadow bundles and reports/refuses conflicts before replacement. Run a reviewed migration for each relevant user first. Once clear, the package backs up existing system bundles, verifies installed payload version/signatures/universal slices/executable SHA-256s, and restores verified backups if post-install checks fail. Package hooks use macOS shell utilities and do not require Python on recipient machines. The developer migration tool requires Python 3, already used by the build/packaging workflow.

A stale `/var/tmp/audio.wolf.tide-install.lock` deliberately blocks a subsequent package install after an interrupted transaction. Inspect its recorded backup rather than deleting it blindly. Native package scripts and Mac metadata/registration behavior have not executed in this Linux session. Portable tests simulate the shell transaction flow and replace macOS-only commands with test doubles; they do not certify real codesign, lipo, Installer.app or AU registration.

No DAW is closed/restarted and no Live set is modified automatically. Rescan or reopen the host when convenient after successful migration; an already loaded 1.2.0 instance can continue using its in-memory code until reloaded.

## Mac build and migration

Extract this release into a new directory, keeping the prior source and installer intact. From that directory:

```sh
cmake -S . -B build-mac -DCMAKE_BUILD_TYPE=Release '-DCMAKE_OSX_ARCHITECTURES=arm64;x86_64' -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0
cmake --build build-mac --config Release --parallel 2
ctest --test-dir build-mac -C Release --output-on-failure
python3 scripts/install-user-mac.py --user-home /Users/isaiahwersky --check
sudo python3 scripts/install-user-mac.py --scope system --user-home /Users/isaiahwersky
TIDE_PACKAGE_ONLY=1 bash scripts/package-mac.sh
```

The migration prints the verified rollback folder and retains `manifest.json` and `auval.log`. To roll back, use the tool from the same verified source tree:

```sh
sudo python3 scripts/install-user-mac.py --rollback '/Library/Application Support/Wolf Audio/Wolf Tide/install-backups/<transaction>'
```

Rollback restores files; refresh/revalidate the restored AU in the logged-in user's session afterward. A first package install without user shadows remains supported. Developer ID signing/notarization still requires configured identities; otherwise the package is unsigned with ad-hoc bundle signatures.

## Validation and limits

The five existing suites remain, plus installer and UI regression suites. The UI coverage includes all 52 combined dropdown programs, all 40 side starters, all 92 preserved portable files, legacy partial imports, independent user names (including names beginning Custom), project recall, model changes, browser save/load, Undo/A-B, and 52 presets across 16 lock combinations (832 recalls). Installer tests cover migration/rollback, corrupt sources/backups, final verification and AU failure, changed destinations, symlink refusal, staged-only signing, package conflicts, valid payload and failed-payload rollback.

Completed here: GCC 14.2 Release VST3/standalone build, 7/7 CTest suites (including the original five), 12 portable installer test cases, and pluginval 1.0.4 strictness 5 with GUI tests (seed `0x7122`). The normal native Linux editor snapshot was inspected, including agreement between User selectors and the footer. Source build ID: `40f5c8b033a9`. Native macOS checks remain pending.

Linux build/test and VST3 validation evidence is recorded under `docs/test-logs/1.2.2/`. `TideDSP.h`, `BloomDSP.h`, `Parameters.h`, and the processor read/prepare/render/processBlock code are byte-identical to the delivered 1.2.1 archive. All 61 parameter IDs, ranges and order, 52 factory program indices, plugin identities and existing preset files remain unchanged. Only metadata, UI/version presentation and installation/packaging behavior change.

Before Mac release, still required: universal native build, native suites and UI inspection, explicit 1.2.2 AU validation, migration of the reported shadow copies, real Installer.app payload/rollback checks, installed file verification and checksums for the resulting package. Native Apple Silicon runtime, DAW session/export testing, signing/notarization and listening claims are not established by the Linux checks. No native 1.2.2 installer has been produced here.
