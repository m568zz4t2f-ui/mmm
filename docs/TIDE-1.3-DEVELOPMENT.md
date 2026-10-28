# Unified Tide 1.3 development — not a release

The user's unified brief is preserved in TIDE-1.3-UNIFIED-BRIEF.md. One coordinated release is intended. No 1.3 installer, release tag, or replacement of the running plugin has been made.

## Frozen baseline

The verified 1.2.2 source was frozen in commit `06d92fb8ef321996b786b04a805206daf342e428` before development. Existing 61 parameter identities and all old processing remain unchanged. The current plugin still compiles as 1.2.2. `Source/ModernDSP.h` is an experimental backend, intentionally not connected to its audio callback or UI yet. This prevents incomplete algorithms from silently changing saved projects.

## Backend experiments implemented

- Twelve delay paths: transparent tap, moving tape head with irregular wow/flutter, clock/companding BBD-inspired echo, independent four-tap timing/levels/pan/feedback, overlapping reverse readers, four-head linked stereo pitch, multi-stage diffusion, owned-history granular capture/freeze, independently quantized/held Lo-Fi, modulated Oil Can diffusion, frequency-band-tapped Spectral delay, and four-head Magnetic Drum timing.
- Thirteen reverb paths: early-reflection/comb Room; coupled dispersive Plate; eight-line Householder Hall; input-diffused Bloom; cross-coupled Chamber combs; sixteen-line ensemble Cloud; pitch-feedback Halo; reflection/pitch Glint; double-pair Studio Plate; sixteen-line Studio Hall; independent input/output-diffused Studio Bloom; finite-window Nonlinear gate; and tuned harmonic comb Resonator.
- Four independent slots, each with a current machine and an outgoing tail. Rapid browsing retires the oldest voice over 40 ms; at most two voices per slot are retained.
- Series, parallel, and complementary low/high split routing, independent levels/bypass, bounded cross-feedback controls and one master dry path. Split is a frequency split, not an undisclosed left/right split. Slot bypass stops excitation while tails continue.
- Reused generation-cleared delay storage and preallocated buffers. Grain freeze stops the capture writer and retains owned audio. Pitch/grain readers use linked stereo trajectories and bandwidth filtering.

These describe actual prototype mechanisms, not professional voicing, physically complete tape/BBD/oil-can models, spectral FFT resynthesis, or hardware equivalence. Shared primitives are still used. Topology independence and numerical differences alone do not establish audible quality.

## Explicitly unfinished

1. `VersionedDSP.h` now provides an experimental preallocated bridge, with a sample-identical legacy regression and an explicit 75 ms revision crossfade. It is not called by the plugin. Integrate the final selected DSP into the processor behind an append-only processing-revision parameter, migrate old states to the frozen path, and prove sample-identical legacy behavior. No new host controls or revision parameter have been shipped.
2. Complete machine-specific control contracts. Prototype settings are not exposed as functioning public controls: for example, dedicated granular tuning/spread, transparent Digital tone modes, diffusion geometry, nonlinear reverse envelopes, reverb lowcut, tap quality, and gate/hold transitions still need focused control-response tests and refinement. Do not expose unused settings.
3. Resolve routing normalization/transition behavior, energy/headroom calibration, correlated-output gain, and all extreme-feedback cases. The current protective output guard is not proof internal feedback is stable. Route changes are experimental; smooth switching and spillover acceptance remain open.
4. Profile costs per machine, four-slot combinations and transition voices, and decide final memory budgets. Preparation allocates both transition voices in every slot; processing sleeps unused slots. No resource/CPU improvement over 1.2.2 is claimed.
5. Retune all factory sounds with representative real guitar, voice, piano/keys, bass, percussion and sustained/chord material, including mono fold-down, wet latency, aliasing, transients and perceived levels. Synthetic signals cannot complete this requirement.
6. Integrate the 1.2.2 selector/identity/lock/undo workflow with all final controls and the redesigned routing panel, then run native host/UI/state regression tests.
7. Build/validate universal AU/VST3/standalone on a Mac, run AU and independent VST3 checks, validate actual installer migration/rollback, and inspect rendered native UI. Mac execution tools are not exposed here.

## Run the numerical lab

```sh
cmake -S . -B build-modern -DTIDE_DSP_ONLY=ON -DTIDE_BUILD_13_LAB=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build-modern --parallel 2
ctest --test-dir build-modern --output-on-failure
```

The optional lab is off by default and does not change the released plugin. Tests exercise 25 paths at 8/44.1/48/96/192 kHz, nonzero/finite responses, within-family pairwise numerical differences at 8 kHz, generation clear, hold input isolation, four-slot routing, dry identity, slot sleeping, processing-allocation instrumentation and parameter automation. Passing those checks is not the 1.3 release gate. Leak detection is disabled in sanitizer runs; no leak-check claim.

## Tools and durable knowledge

GitHub operations use the existing authenticated Git HTTPS connection; main is not modified. Graphify tools are not exposed, so these notes have not been ingested, approved or indexed there. Future Graphify work must verify the indexed commit and keep experiments separate from completed implementation and musical findings.

## Current measured checkpoint

The optional DSP-only lab passed 7/7 registered suites, including the two new lab suites; the unchanged full plugin configuration separately passed its existing 7/7 suites. Allocation instrumentation observed zero allocation calls during the tested processing and parameter-update sequences at 44.1/48/96/192 kHz. Prepared lab payload was 43,495,392 bytes (41.5 MiB) at 48 kHz and 173,938,528 bytes (165.9 MiB) at 192 kHz, across all four slots and their transition voices. These exclude a future simultaneous legacy engine and are heap payload, not resident-memory measurements. No CPU deadline or blanket optimization claim follows from these figures.

See `docs/test-logs/1.3-lab/` for exact results and the development notes above for what remains unimplemented. This is a numerical backend checkpoint, not completion of the unified brief.

## Recorded-fixture analysis

The new optional `tide_modern_audio` utility uses the existing JUCE reader to analyse four seconds of input plus two seconds of tail per machine. It rejects empty/invalid-rate fixtures. It processed the pinned JUCE examples' `cello.wav` and `singing.ogg` through all 25 paths: 50 nonzero finite renders. Input audio and output audio are not redistributed. These are cello and vocal recordings, not guitar recordings; `guitar_amp.wav` was not mislabeled as a played guitar performance.

Observed direct wet peaks reached 1.14721 for Digital on the vocal fixture. Delay integrated wet/input energy ranged from -19.03 to +0.51 dB; reverb from -17.24 to +1.68 dB. These are energy/peak figures for short fixtures, not LUFS or listening-quality scores. They establish a real gain/duty-cycle calibration problem and block release. They do not justify one universal gain boost. The optional master guard does not substitute for fixing machine gain/feedback behavior.

Build with `-DTIDE_BUILD_13_LAB=ON` in a full JUCE build and run `tide_modern_audio /path/to/recording.wav /path/to/recording.ogg`. More varied instrument/reference material and actual audition remain necessary. The tests and new backend are checkpoints in the one-release project, not completed public features.

The final full build with the lab enabled passed 9/9 suites (the seven existing suites plus the two numerical lab suites). AddressSanitizer/UndefinedBehaviorSanitizer passed the lab harness, including the legacy bridge comparison; leak detection was disabled. No native Mac or listening validation follows from this result.
