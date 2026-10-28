# Wolf Tide 1.3.0 — source and Mac installer handoff

Status: **integrated source build candidate**, October 28, 2026. Compiled source build identifier: `866d3cc79c34`. This delivery has no macOS binaries, completed native Mac acceptance or human listening approval. It does not claim completion of those release gates.

## Included

Twelve delay and thirteen reverb paths, delay A/B and reverb A/B, three routing modes, independent B controls/sync, cross feedback, held capture/reverb, two-voice spillover, fixed per-model return calibration, native dynamic controls, compatible project state and preset workflow, 64 combined programs and 50 side starters. The 104 new controls are appended after the original 61 parameters. All 92 previous portable presets remain; 114 new files are under `Factory-Presets/1.3`.

The Source ZIP includes complete Tide source, tests, docs, installer migration/rollback/package scripts and pinned JUCE 8.0.12 source. Run `bash scripts/Build-Mac.command` on the Mac; see [the build guide](BUILD-MAC-1.3.md) for prerequisites, old user/system copies, AU validation, signing/notarization and rollback. Packaging always rebuilds incrementally and runs tests before staging corresponding source.

## Completed verification

- Release Linux VST3/standalone build with GCC 14.2 and pinned JUCE.
- Eleven registered CTest suites pass. DSP coverage includes all 25 paths at 8/44.1/48/96/192 kHz, finite/nonzero output, reset, held capture, long taps, routing, allocation instrumentation and outgoing-tail calibration.
- Actual plugin callback tests render all 25 paths and exercise four-slot routing, schema-4 recall, old-state defaults, scoped upgrades and locks.
- Native UI tests cover 64 combined/50 side dropdown paths, 206 portable file imports, independent identities, 1024 lock combinations, browser load/save, undo/A-B, sync-time text, advanced page changes and host-automated machine-specific controls. Candidate screenshots were inspected on Linux.
- pluginval 1.0.4 strictness 5 **with GUI tests**, seed `0x1300`, requested rates 44.1/48/96/192 kHz, succeeds on the candidate VST3. This is Linux validation, not AU or Mac host validation.
- AddressSanitizer/UndefinedBehaviorSanitizer pass the current DSP harness, including legacy bridge and per-voice tail calibration. Leak detection was disabled; no leak-check result is claimed.
- A renderer compiled separately against frozen 1.2.2 source and the compatibility header produces identical 39,135,096-byte output streams across 99 model pairs/four rates, including delay-time changes and bypass. Both streams' SHA-256: `f00d597829046b1260473ad1ce7792b8114799fcca5967365d2c80cfd5483974`. This establishes the tested fixture's compatibility, not cross-platform hash identity.
- All 337 tested machine/control extrema contracts respond. Twelve portable installer transaction/package-hook tests pass; native Installer.app behavior is not established by those mocks.

Exact logs/measurements are in `docs/test-logs/1.3-candidate/`. Earlier `1.3-lab` records describe the previous disconnected checkpoint.

## Recording and level evidence

The pinned JUCE cello and singing examples were processed through all 25 bare machine paths (50 finite, nonzero renders; four seconds input plus two seconds tail). No derived audio is distributed. The utility reports raw and calibrated energy, peaks and stereo correlation. It is not a perceptual loudness meter or a listening test.

Fixed return trims target approximately -2 dB mean wet/input energy for delay and -6 dB for reverb over those two fixtures at the utility's stated settings. **The two-fixture mean is not a guarantee on either source or other settings.** Individual calibrated figures span -3.59 to -0.47 dB for delay and -11.69 to -0.34 dB for reverb. Direct calibrated return peaks reach 1.251 (delay) and 0.948 (reverb), before Amount/Mix and the wet-active emergency ceiling. These limits matter for audition and gain staging; no claim of universally matched perceived levels follows.

## Resource budget

Processing/model-update allocation instrumentation observes zero tracked allocations in its bounded fixtures at four standard rates. Preparation allocates the modern transition pool: 43,495,392 bytes at 48 kHz and 173,938,528 bytes at 192 kHz, plus the compatibility engine/JUCE/host. No reduced-memory claim is made. Slots sleep when inactive; retaining allocation avoids callback resizing.

The measured four-slot fixture takes about 0.09–0.10 seconds elapsed wall time per second of audio at 44.1 kHz, 0.10–0.12 at 48 kHz, 0.22–0.25 at 96 kHz and 0.54–0.65 at 192 kHz on this shared Linux runner. This is a short reproducible profile, not worst-case scheduling/deadline certification. Eight simultaneous transition voices, different patches, hybrid legacy processing and Mac hardware can cost more. Establish the intended native session budget before distribution.

## Required local acceptance

Build universal AU/VST3/standalone, validate the actual 1.3.0 AU, run Mac VST3 checks, inspect the native UI and reopen legacy/new DAW sessions. Validate both architectures, installer payloads, duplicate-install migration and rollback. Apply Developer ID signing/notarization for public distribution using your own credentials.

Audition varied instruments and levels, transients, chords, mono fold-down, timing changes, pitch artifacts, decay/hold, routing transitions and feedback extremes. Factory recipes were revised and numerically exercised; broad instrumental voicing and professional musical acceptance remain unverified. The documented candidate and evidence enable those checks; they do not replace them.

Main and the live Mac installation remain unchanged. No release tag was created. Graphify tools were unavailable; no indexing/approval is claimed there.
