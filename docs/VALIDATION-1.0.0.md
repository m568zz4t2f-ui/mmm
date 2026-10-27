# Wolf Tide 1.0.0 — validation record

Built on Linux x86_64 and macOS 15.7.9 (Intel). The macOS plugin bundles contain **x86_64 and arm64** slices. Only the Intel slice was executed on the connected Mac; Apple Silicon execution remains untested.

## Completed

- Release DSP suite: six delay modes at 44.1/48/96 kHz, four reverb models, clean-delay sample timing, hard-panned stereo preservation, ping-pong cross-feedback, mono fold-down, independent delay-to-reverb feed, bypass tails, Send dry suppression, explicit clearing, automation and maximum-shimmer tail decay. Final Linux CTest: 2/2 passed, 23.28 seconds. Mac final CTest: 2/2 passed, 13.64 seconds.
- Processor/editor harness: mono, stereo and mono-to-stereo; transport reset retains sound; prepare clears sound; all 43 parameter names are unique; all 16 factory combinations match their intended parameters; session edits/locks recall; Mix lock protects all three blend controls; side preset scope and malformed/nonfinite-file rejection; editor creation and all three expanded panels; tempo-synced display shows actual milliseconds. Final Mac host suite includes the display assertion.
- AddressSanitizer + UndefinedBehaviorSanitizer: full pre-polish DSP suite passed. Latest DSP additionally passed the boundary harness at 8/48/192 kHz, including 8-second main / 16-second secondary buffers, wrapping, rapid time edits, invalid audio inputs, all models and clearing. LeakSanitizer was disabled because this environment's process isolation prevents its thread inspection. No leak-check claim is made.
- Independent Linux VST3 audio/API checks: Tracktion pluginval 1.0.4, strictness 5, SUCCESS. GUI-window tests were skipped: this headless environment cannot establish a usable native X11 display. A GUI-enabled attempt crashed at native-window creation; this does not count as a GUI pass. Software component snapshots and the processor/editor harness passed separately.
- macOS Audio Unit: installed in the current user's Components folder and `auval -v aufx Wtid Wolf` completed with **AU VALIDATION SUCCEEDED**. Initial discovery needed an AudioComponentRegistrar refresh after installation.
- Native AU and VST3 bundles installed in the connected Mac user's plugin folders. Existing Wolf Bloom was preserved. No DAW was closed or modified.
- Standard macOS `.pkg` generated with pkgbuild. Ad-hoc plugin signatures verified during packaging. Installer includes AU, VST3, standalone app, quick guide, license and complete corresponding source, including pinned JUCE. No Developer ID identities were present; this installer is unsigned and not notarized.
- Native vector interface snapshots inspected at the default size and in all deep editors. Synthetic WAV examples are included; these are not real instruments or reference-hardware comparisons.

## Important limits

This is a first development release, not a certified commercial emulation. Factory presets are designed starting points and numerically checked, but have not been optimized against recorded vocals/guitar/piano in a full mix. Tape/Bucket are character models, not complete measured circuit/transport replicas. Hall/Bloom share an FDN foundation. No exact Strymon or Eventide sonic match is claimed.

A per-user plugin installation and AU validation were completed. A clean-machine, administrator-level installation/upgrade/uninstall of the actual `.pkg`, Apple Silicon execution, Ableton Live session automation/reopen/export, and live listening/CPU evaluation remain release checks. Tails require the DAW to keep processing; no plugin can continue after its host stops the audio engine. Freeze may require manually extending export tails.

## Reproduce

Normal release build: CMake configure/build, then CTest. Optional boundary checks:

```sh
c++ -std=c++17 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -I Source tests/safety_tests.cpp -o tide-safety
ASAN_OPTIONS=detect_leaks=0 ./tide-safety
```

Packager defaults to building and testing. `TIDE_PACKAGE_ONLY=1` repackages already validated binaries after documentation-only changes; it does not validate or rebuild them. Logs in `docs/test-logs` record completed local checks.
