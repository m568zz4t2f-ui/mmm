# Wolf Tide 1.2.1 repair update

Prepared from repository main `bcd5369016fb71fd2ceaaae4bfe452d9060fdf30` (the consolidated Tide 1.2 source plus the separate Prism archive). Tide files alone are changed. The Mac installation described in the 1.2 handoff has not been updated.

## Repaired audit findings

| Audit finding | Change in 1.2.1 |
|---|---|
| A: mono ping-pong | Mono launches into the left lane and crossed feedback returns to the right. Genuine stereo retains its side information. Tests cover launch ordering in all nine models and alternating timed repeats in simple echoes. |
| B: dead Halo/Studio Plate Size and Motion | Boutique plates now scale and modulate their actual tank reads and output taps, with smoothed geometry/motion and decay compensation. Classic Plate retains its original read path. Control tests run at 8, 48 and 192 kHz. |
| C: bloom retriggers suppress tails | Glint and Studio Bloom use input allpass buildup instead of multiplying the complete tank output by a retriggered envelope. Short impulses excite the buildup rather than being discarded. Different buildup coefficients distinguish unpitched Glint from Studio Bloom. |
| D: ignored independent B timing | Pattern and Reverse use channel-specific manual/synced durations. Linked ratio also reaches the reverse heads. |
| E: incomplete time transitions | Pattern crossfades complete tap sums. Reverse maintains old/new trajectories per channel and crossfades their complete output. Pending requests coalesce until the current transition finishes. |
| F: shared diffusion ownership | New `dDiffusion` controls the Diffusion delay. Reverb diffusion stays on the reverb side. Older combined states/presets migrate the former shared value once; old delay-only presets use .7 because they never stored reverb diffusion. Reverb recalls never rewrite delay diffusion. |
| G: hidden legacy coloration | One resolved model determines delay dispatch and coloration; extension models no longer inherit hidden Digital/Tape/Bucket filtering. |
| H: inverted swell behavior | A threshold/hysteresis detector drives an actual rising attack (10–1510 ms) with an 80 ms quiet retrigger interval and 50 ms release. Tests compare early/late energy on sustained input. |
| I: stale UI capability checks | Model resolution is shared with DSP. Unsupported classic reverb advanced controls are disabled; boutique plate Size/Motion are enabled. Reverse Motion/Rate are disabled. B timing follows Independent/Sync, and Pattern tap selection follows the active modern Pattern path. |

## Additional improvements

Granular Pitch now has eight preallocated stereo grain slots, individual Hann envelopes and bounded read trajectories, instead of two continuously running pitch shifters. The main interval and companion interval remain distinct at +24. Motion adjusts overlap/source spread; Motion Rate adjusts grain duration. Source history and anti-alias filtering are retained inside the scheduler; tests cover all six intervals at 8/48/192 kHz, channel isolation and reset. This is an original granular design, not a hardware reconstruction or an alias-free pitch claim.

Unused boutique plate FDN buffers and unpitched-model pitch buffers are no longer allocated. The prepared engine requests about 18.26 MiB at 48 kHz and 73.00 MiB at 192 kHz, including the new grain/buildup storage (the 1.2 audit reported 17.49/69.91 MiB). This trims 1.59/6.34 MiB from the initial repair implementation; it is not a memory reduction against 1.2.0. The sampled Digital processing audit reports zero allocation calls.

Settled modern pitch filters stop rebuilding coefficients. Independent B feedback/level, delay diffusion and boutique blend/regeneration/attack controls are smoothed. No new DSP framework was added.

Session and preset parsing reject malformed numbers, fractional choices, duplicate known parameters and incomplete current schemas before applying settings. New state/preset schema 3 retains schema 1/2 readers. Source packaging now includes the pinned JUCE source even when CMake fetched it; missing source or a wrong Git revision fails packaging. Mac installation/package scripts now target 1.2.1.

## Compatibility

All original 60 parameter IDs, indices, ranges, program indices and plugin identities remain intact. `dDiffusion` is appended at index 60 (61 parameters total). The 52 combined programs and 40 side starters remain. The host suite checks extension ordering, legacy migration, scope/locks, state recall and visual/audio independence.

This is behavioral repair, not sample-identical recall of 1.2 audio: ping-pong, swell, boutique plates, bloom and Granular Pitch sound different. Preserve the 1.2 installation and rendered stems before upgrading critical sessions. Classic Plate's integer tank path remains available. New schema-3 presets should be loaded with 1.2.1; older binaries cannot be assumed to understand them.

## Validation in this Linux environment

- GCC 14.2, Release, pinned JUCE 8.0.12 (`29396c22c93392d6738e021b83196283d6e4d850`). VST3 and standalone built.
- Five CTest suites passed: dsp, update, models, repairs, host. Native host tests include mono/mono, mono/stereo, stereo/stereo, reset/tails, preset migration, UI rendering and factory export.
- pluginval strictness 5 passed, seed `0x7121`, including editor and editor-while-processing checks, default sample rates/block sizes. It identified Wolf Tide v1.2.1. No GUI tests were disabled.
- ASan/UBSan passed the DSP safety and repair harnesses. Leak detection disabled; no leak-check claim.
- Native normal, delay, reverb and routing editor snapshots exported; delay deep-edit layout inspected.
- Performance CSVs retain the existing harness's six scenarios (stereo 48 kHz, 128-sample blocks, 1500 retained blocks; median/P99/max microseconds). They are single before/after passes in a shared cloud machine. Medians are broadly similar and observed scheduling peaks vary; no blanket CPU improvement or realtime guarantee is claimed.

Logs are under `docs/test-logs/1.2.1/`. Historic audit logs remain evidence about 1.2.0, not new failures or proof of 1.2.1 behavior.

## Still outstanding

A new macOS universal build, Apple AU validation, native Apple Silicon execution, Mac installer validation/install/rollback, DAW session/export tests, signing/notarization and human audition have not run here. Available tools provide no connected Mac execution. On a Mac, build/test/package with `bash scripts/package-mac.sh`; `python3 scripts/install-user-mac.py` installs validated 1.2.1 bundles to the per-user locations used previously and retains rollback copies. Check both user and system plugin folders for shadowing; the package remains system-wide and has no verified shadow-copy cleanup in this update.

Physical BBD/tape reconstruction, exact proprietary algorithms, matched hardware comparisons, calibrated model loudness/decay, editable per-tap sequencing, extended soak/automation coverage, all-input stability proof and exhaustive peak-CPU guarantees remain separate work. Cloud stays the existing original FDN; no CloudSeed/Signalsmith code was imported. Presets were exported from working code but were not retuned by human listening. These limits prevent calling every musical/platform issue solved.
