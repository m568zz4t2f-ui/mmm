# Tide 1.2 execution handoff — start when the user says “go”
Prepared: 2026-10-27.
State: preparation complete; DSP/UI changes have not started. No 1.2 binary or installer exists.
Scope of “go” in this conversation: begin implementing the agreed update, validate it, and produce the normal shareable package. Preserve the installed working version and follow existing installation authorization if still applicable. This handoff does not launch a background job.

## Read first; do not repeat the broad research
1. docs/TIDE-1.2-UPDATE-SPEC.md is the full scope, design rationale, research links and release gates.
2. docs/PROJECT-MEMORY.md has the baseline and corrected findings.
3. Inspect current source/diff before edits; these files describe the checked 1.1.0 snapshot, not every possible later change.
4. Use GitHub for durable source/checkpoints, Graphify for additional context (candidate notes are unverified), and Ponytail for the smallest correct implementation. Do not let unrelated Cosmos/Prism notes redefine Tide.
5. Create a dedicated implementation branch/checkout from the latest agreed baseline; preserve uncommitted work and installed 1.1.0. Do not overwrite a newer user change.

## Where to work
- Durable repository: https://github.com/m568zz4t2f-ui/mmm
- Current working source: /workspace/scratch/ef6a0a07b665/WolfTide (transient; verify before relying on it).
- Previously reported Mac source: /Users/isaiahwersky/Downloads/WolfTide-1.1.0 (not reverified during this preparation).
- Cosmos reference: /workspace/scratch/f269aa6090a9/WolfCosmos-0.2.0/Source/CosmosDSP.h (verify availability).
- Mac packaging entry: scripts/package-mac.sh. There is no scripts/build-mac.sh.
- Current CMake project and packaging script both declare 1.1.0. Update them together at release.
- Pinned JUCE: 29396c22c93392d6738e021b83196283d6e4d850; local vendor/JUCE-8.0.12.tar.gz exists. GitHub is a source snapshot without the vendor archive/binary assets; confirm complete inputs on the actual build machine. Do not assume the cloud checkout is an offline distribution.

## Implementation queue and source map
Each checkpoint must compile and pass relevant checks before continuing; preserve one release scope rather than asking the user to re-explain it.

1. Fix timing and establish compatibility fixtures.
   - Source/TideDSP.h: trace Echo::set, Echo::tick and Engine::set callers. Apply coalesced pending delay targets at processing transition boundaries.
   - tests/update_tests.cpp: add the smallest regression that fails on 1.1.0 and passes the fix, including a request received during a fade. Extend only for meaningful new edge cases.
   - tests/host_tests.cpp + Source/PluginProcessor.cpp: legacy state and normalized automation fixtures before new engine selectors.
   - Important correction: do NOT “fix” Air Throw pitchMode reset. Float defaults do not include that choice parameter.
2. Improve pitch and audit routing/tails.
   - Source/TideDSP.h: stable active trajectories/windows, boundary-safe interval changes, smoothed filter transitions, linked timing with independent channel states.
   - Source/PluginProcessor.cpp: prepare/reset lifecycle and fixed/climbing state. Confirm same-rate repeated prepare behavior before changing it.
   - Retain old sound behavior through versioning. Measure wet onset/repeat spacing, high-interval aliasing, chords/transients and long buffered repeats.
3. Everyday delays first.
   - Source/TideDSP.h: clean Digital, Tape and Analog/Bucket refinements; independent Dual A/B configuration; bounded Pattern taps; reusable duck/swell.
   - Source/Parameters.h + processor layout/state: compatibility-safe extension parameters and explicit recipes, no reordered legacy values.
   - Slapback, dotted eighth and quarter note are easy recipes/options, not redundant engines.
4. Reverb models.
   - Source/TideDSP.h and Source/BloomDSP.h; introduce a focused DSP file only when needed for clear ownership.
   - Refined Plate/Hall/Bloom, Chamber, Cloud Ensemble, dedicated Halo Shimmer and Glint. Preserve legacy tanks.
   - Diffusion and bounded Granular follow the core sounds. Aurora spectral backend, Lo-Fi, Spring and Resonator remain gated by the specification; do not silently substitute a preset for a promised distinct algorithm.
5. Presets and UI.
   - Source/Parameters.h + PluginProcessor.h/.cpp: explicit complete recipes, stable IDs/schema, scoped validation, direct user save, undo and locks.
   - Source/PluginEditor.h/.cpp: one shared browser in three scopes, search/tags, Favorites/Recent, edited/partial state. Keep grey/blue boutique design and cached graphics.
   - Curate 32 combined musical +4 utility presets and two side starters per shipped engine; these numbers are targets, not existing curated assets.
6. Optimize, validate and package.
   - Benchmark the same fixture/rate/block settings before and after each consequential DSP change. Capture typical AND peak block time, allocations, memory, and UI open/closed cost. Do not call elapsed test runtime a DSP benchmark.
   - Group coefficient updates, process only needed engines/voices, bound outgoing transitions, retain safe sleep horizons. No callback I/O, allocation or locks.
   - Release old state, normalized automation, stereo/mono fold-down, bypass/send/full-wet, long-tail/freeze/clear, silence wake, preset scope/undo and rapid switching checks.
   - Then version/build/package/install smoke-test; keep rollback and corresponding source/licenses. Report actual architectures/signing status.

## Confirmed pending-target reproduction
8 kHz, unsynced time=100 ms, stereo=0, feedback=0, motion=0, lowcut=20, tone=16000. Prepare Echo and set parameters. Request 200 ms, process 80 silent samples, request 300 ms, process 800 silent samples, clear history without resetting target/tap time, then inject a left impulse and find the first return above 0.001.
Observed on checked 1.1.0: requested_ms=300 first_repeat_ms=200.
Regression intent: latest target is eventually honored; old fade is not continually restarted. Account for any deliberate algorithm latency in additional pitch tests.

## Verified preparation checks
Run during preparation, on Linux with c++ -std=c++17 -O2:
- tests/dsp_tests.cpp: PASS (timing, six delays, four reverbs, stereo, shimmer, bypass tails, routing/send, clear, automation).
- tests/update_tests.cpp: PASS (logical clear, conditional processing, wake, 16-second echoes, mute, transitions, +24 anti-alias rejection reported -26.5702 dB).
- bash -n scripts/package-mac.sh: PASS.
- CMake configure: NOT RUN successfully; cmake is not installed in this workspace.
- JUCE host tests, AU/VST3 builds, target-hardware profiling, listening comparisons and installer tests: NOT performed during preparation.

These existing tests do not cover the newly confirmed pending-target bug and do not establish professional sound quality.

## Commands for the execution session
DSP-only using the currently available compiler:
```sh
c++ -std=c++17 -O2 -I Source tests/dsp_tests.cpp -o /tmp/tide-dsp-tests
/tmp/tide-dsp-tests
c++ -std=c++17 -O2 -I Source tests/update_tests.cpp -o /tmp/tide-update-tests
/tmp/tide-update-tests
```
With CMake available, a reproducible DSP-only build:
```sh
cmake -S . -B build-dsp -DTIDE_DSP_ONLY=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build-dsp --parallel 2
ctest --test-dir build-dsp --output-on-failure
```
On the verified Mac build host, after fixing release versions and checking dependencies:
```sh
bash scripts/package-mac.sh
```
The packaging script builds AU/VST3/Standalone for arm64+x86_64, runs its registered tests, signs bundles with the configured identity or ad hoc, includes source/licenses/docs and builds a pkg. Universal output, host validation, upgrade behavior and actual signing/notarization must still be verified; the script's requested options are not proof of success.

## Required final deliverables
- Implemented source on a named GitHub branch/commit; updated compatibility/schema notes.
- Actual check/benchmark results with limitations, preset inventory and versioned release notes.
- Shareable Mac installer with honest architecture/signing status, corresponding source/licenses, and rollback instructions.
- Graphify notes for durable decisions and corrected findings; never report a queued note as approved.
