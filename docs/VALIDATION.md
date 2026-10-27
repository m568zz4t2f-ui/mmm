# Wolf Tide 1.1.0 — validation record

## Completed on this update

- Linux Release CTest: 3/3 passed, 10.50 seconds. Includes six delay families, four reverb families, routing/timing/stereo, maximum-shimmer decay, bypass tails and automation.
- macOS Release CTest: 3/3 passed, 5.45 seconds. Native processor/editor tests include all 44 unique parameters, complete factory state, scoped presets, legacy preset pitch routing, state/locks, stereo layouts, and sample-identical output with different visual modes and visible-meter collection.
- Update regressions: logical clear across block boundaries/wraps; one selected reverb runs; unused shimmer sleeps; silent enabled engines sleep and wake; 8/16-second delays survive silent gaps; unconnected zero-amount returns suspend; outgoing reverb tails remain; cleared state never resurfaces; Fixed Harmony produces finite stereo-linked repeats; +24 pitch filtering rejects the tested 10 kHz input by 26.57 dB relative to the tested 2 kHz input. This is a narrow numerical check, not a universal listening-quality score.
- Latest DSP safety harness passed under AddressSanitizer and UndefinedBehaviorSanitizer at 8/48/192 kHz, long buffers, wrapping, invalid input, rapid edits and clearing. LeakSanitizer disabled for this environment's thread-inspection limitation; no leak-check claim.
- Latest Linux VST3: pluginval 1.0.4, strictness 5, audio/API SUCCESS. GUI-window checks skipped in the headless validator; native component snapshots and processor/editor checks passed separately.
- macOS AU/VST3/Standalone Release bundles built with x86_64 and arm64 slices. Only the architecture executed by this connected Mac was runtime-tested; a separate Apple Silicon machine was not tested.
- Version 1.1.0 AU and VST3 installed in the current user's plug-in folders; standalone installed in ~/Applications/Wolf Audio. Previous bundles preserved under ~/Downloads/WolfTide-1.1.0/install-backups/20261026-190543. Installed signatures verified. Native Audio Unit check completed with AU VALIDATION SUCCEEDED after registrar refresh.
- All three expanded interface panels rendered and inspected on Linux; native Mac editor snapshots/export checks also passed. Updated portable factory files exported. Three 18-second stereo synthetic examples were rendered and checked for complete lengths, headroom and final zero samples; they are not real instrument/hardware comparisons.

## Measured DSP performance

On the connected Mac, identically compiled C++17 O3 harnesses processed 3 seconds of 48 kHz stereo audio. Each invocation takes a median of three trials; three before/after invocations alternate order. Comparison excludes prepare/allocation, graphics and DAW overhead. These figures are reductions in harness processing time, not promised reductions on Live's CPU meter.

| Configuration | Reduction vs 1.0.0 |
|---|---:|
| Tape + Room | 72.3% |
| Tape + Plate | 75.2% |
| Tape + Hall | 62.3% |
| Tape + Bloom | 60.7% |
| Tape + Hall + Shimmer | 49.8% |
| Reverse + Hall | 64.2% |
| Pitch + Hall + Shimmer | 42.9% |
| Both effects off, silence | 96.9% |

Raw data and the method are in docs/test-logs/mac-benchmark-1.1.0.json. Reproduce with tests/benchmark.cpp compiled against each source revision and tests/compare_benchmarks.py. Peak callback latency, UI CPU/GPU measurements and real DAW poly-instance workloads were not measured by this harness.

## Distribution and limits

The standard Mac packager includes AU, VST3, standalone, guide, license, portable presets and complete corresponding source with pinned JUCE. No Developer ID signing identities are available; plug-ins are ad-hoc signed and the installer is unsigned/not notarized. A clean-machine administrator-level package install/upgrade/uninstall remains untested.

This improves the existing six delay/four reverb families. It does not add the larger proposed multi-algorithm library, physical BBD/tape, dispersive/modal physical plates, spectral/formant pitch, or a certified Strymon/Eventide emulation. Granular texture and internal wet pitch latency remain, especially inside climbing feedback. Room/Plate/Hall/Bloom and delay voicing changes can change recalled sounds even though existing parameter IDs/order are preserved; the previous installed version is backed up.

No live Ableton session/listening/export, blind comparison on recorded vocals/guitar/piano, separate Apple Silicon execution, or calibrated GPU profiling was completed. Stop retains tails only while the host continues processing. Hold can need extended export tail time. Factory presets remain designed starting points.

## Reproduce

Release build: CMake configure/build, then CTest (use a display/Xvfb for the Linux editor harness). Safety build:

```sh
c++ -std=c++17 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -I Source tests/safety_tests.cpp -o tide-safety
ASAN_OPTIONS=detect_leaks=0 ./tide-safety
```

Packager builds/tests by default. TIDE_PACKAGE_ONLY=1 packages already validated binaries after documentation-only changes. Validation from the original release is retained separately in VALIDATION-1.0.0.md.
