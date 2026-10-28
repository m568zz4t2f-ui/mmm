# Wolf Tide 1.3 — integrated source candidate

The unified specification is preserved in [TIDE-1.3-UNIFIED-BRIEF.md](TIDE-1.3-UNIFIED-BRIEF.md). The former optional backend checkpoint has been integrated into the plugin. This branch builds as 1.3.0; it is a candidate for native Mac build and musical acceptance, not a validated macOS release.

## Implemented in the plugin

- Twelve delay and thirteen reverb paths with documented processing mechanisms. Halo uses a dedicated twelve-line pitch-feedback network; the specialized paths are not merely renamed `BoutiqueTank` configurations. Shared storage/filter/diffusion/pitch primitives remain appropriate. See [algorithm and control details](ALGORITHMS-1.3.md).
- Independent delay A/B and reverb A/B, Series/Parallel/Frequency split, independent B sync/rhythm, levels, filtering, modulation, bypass, held audio and bounded cross feedback. Each slot has two prepared voices; outgoing tails retain their own return calibration.
- Fixed per-model return calibration measured on the documented cello/vocal fixtures; calibration stays outside feedback loops. A wet-active soft emergency ceiling handles excessive final output. No automatic loudness matching or proprietary emulation is claimed.
- Actual processor/host parameter integration: 104 appended controls, 165 total. The first 61 IDs, normalized domains and ordering are unchanged.
- Schema-4 project and scoped preset saving, old schema migration to Legacy/B-off defaults, per-family processing revisions and transitions. The complete Legacy path uses the original arithmetic. An optional stem interface supports mixed old/new sections.
- Native scrollable A/B and routing editor, dynamic machine-specific controls, tempo/dual/revision-dependent enablement, and explicit Legacy/Tide 1.3 selectors. UI model changes and automation refresh through the same state.
- 64 combined programs (16 compatibility, 48 revised/new), 50 side starters, 114 new portable files and 92 preserved files. Independent identities, Factory/User/Modified labels, locks including B returns/routing, undo-load, comparison, favorites and search remain supported.
- Universal Mac build/package command, pinned dependency source in the delivery ZIP, source bundling, signature/architecture/version checks and verified duplicate-install migration/rollback tools.

## Validation

The integrated candidate has passed all eleven suites, including actual callback rendering of every new machine, all preset/UI paths, old portable file migration, 1024 combined lock configurations, schema-4/scoped state recall and live advanced-panel automation/page changes. The control harness exercises 337 machine/control extrema combinations. The latest delivery evidence and its exact source build identifier are recorded in [RELEASE-1.3.0.md](RELEASE-1.3.0.md).

Numerical DSP tests cover 8/44.1/48/96/192 kHz, nonzero/finite responses, within-family response differences, logical reset, held capture isolation, long silent-gap taps, routing, output bounds and outgoing-tail calibration. Allocation instrumentation checks model/routing updates and processing at the four standard rates. Passing a bounded fixture is not a proof of every possible feedback trajectory or a listening-quality score.

A separate renderer compiled against frozen 1.2.2 commit `06d92fb8ef321996b786b04a805206daf342e428` and the compatibility header produced identical 39,135,096-byte output streams across 99 delay/reverb combinations at four rates. The fixture includes delay-time changes and host bypass. `tests/legacy_compat_render.cpp` preserves the renderer for same-toolchain comparisons; cross-platform math-library hashes need not match.

## Release acceptance still required on the Mac

Universal native build/tests, both architecture/runtime checks, explicit 1.3.0 AU validation, independent Mac VST3 validation, standalone/DAW session and export checks, Installer.app payload/rollback acceptance, and distribution signing/notarization. The earlier report of user/system duplicate installs has not been reverified or changed here. Follow [BUILD-MAC-1.3.md](BUILD-MAC-1.3.md).

Human audition on varied guitar, voice, piano/keys, bass, percussion, transients and sustained chords remains required. The two short recording fixtures establish return-level/correlation/peak measurements, not LUFS matching or professional voicing approval. Factory recipes have been revised and numerically exercised; they have not been auditioned on all those instruments. Pitch/grain windows retain audible texture on some material. Do not call the unified brief's musical/native release acceptance complete from these tests alone.

Prepared transition pools trade memory for allocation-free switching. Modern payload alone is about 41.5 MiB at 48 kHz and 165.9 MiB at 192 kHz, plus legacy/host overhead. Disabled modern slots sleep; hybrid compatibility can cost more CPU than all-modern operation. Profile the intended Mac sessions before setting a deployment budget.

GitHub main is unchanged; no release tag or Mac installer is published from this branch. Graphify tools were unavailable, so this status has not been indexed there. Historical lab evidence is retained under `docs/test-logs/1.3-lab`; use the candidate handoff for the current implementation.
