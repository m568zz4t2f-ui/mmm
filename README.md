# Wolf Tide 1.2.0

See [release notes](docs/RELEASE-1.2.0.md) for models, browser, compatibility and known limits. See [current validation](docs/VALIDATION-1.2.0.md) for checks actually performed.

# Original project guide

**Wolf Tide 1.1.0 · Echoes into atmosphere.** Stereo delay + reverb by Wolf Audio.

Grey delay on the left, blue reverb on the right. Six delay modes, four reverb models, independent presets and bypass, 16 complete factory combinations, pitch/shimmer, separate ducking sensitivity/release, tempo sync, a Bloom performance macro and one contextual editor. The boutique grey/blue interface combines native vector controls and cached printed artwork. Static, Activity and Motion display modes, precise value entry and a resizable layout are included. Inactive tanks and shimmer suspend safely; outgoing reverb tails continue. Pitch delay offers Fixed Harmony and Climbing Repeats.

**Release status: development build.** Current completed platform builds and checks are recorded in `docs/VALIDATION.md`. The Mac installer, when supplied, contains universal AU/VST3 bundles. A source archive or Linux plugin is not a Mac installer. Developer ID signing/notarization and real-source DAW listening are separate release gates.

## User guide

Read `docs/QUICK-START.md`. Presets save as portable `.wolftide` files, separately for delay, reverb or a whole combination. Factory sounds cover guitar, vocals, piano, electric keys and ambient textures. They are designed starting points, not a promise of automatic optimization for every recording.

## Build

The complete pinned JUCE 8.0.12 source is bundled. CMake 3.22+ and a C++17 compiler are required on the build machine only.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel 2
ctest --test-dir build -C Release --output-on-failure
```

On Linux, the host/editor test needs a display (use `xvfb-run -a ctest ...` headlessly). Required development libraries are listed in `.github/workflows/build.yml`. AU builds only on macOS. VST3 and standalone builds are enabled on Linux/macOS; Windows is not validated in this delivery.

For dependency-free DSP tests:

```sh
cmake -S . -B build-dsp -DTIDE_DSP_ONLY=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build-dsp --parallel 2
ctest --test-dir build-dsp --output-on-failure
```

## Produce the Mac installer

On a Mac build machine with Apple command-line tools and CMake:

```sh
bash scripts/package-mac.sh
```

This builds universal arm64/x86_64 AU/VST3 and a standalone app, runs automated tests, and creates an explicitly **unsigned** installer under `dist/`. Recipients of a completed installer do not compile or run this script.

For Developer ID distribution, supply `TIDE_APP_IDENTITY`, `TIDE_INSTALLER_IDENTITY` and a previously configured keychain `TIDE_NOTARY_PROFILE`. The script verifies signatures, submits for notarization, staples and assesses the completed package. It does not acquire an Apple developer account or create signing credentials. Without these identities the output remains ad-hoc-signed/unsigned, with macOS security prompts expected. The included workflow prepares unsigned macOS artifacts; its existence is not evidence the build ran.

The installer includes the complete corresponding source and license under `/Library/Application Support/Wolf Audio/Wolf Tide/`, AU under `/Library/Audio/Plug-Ins/Components`, VST3 under `/Library/Audio/Plug-Ins/VST3`, and the standalone app under `/Applications/Wolf Audio`.

## Validation and research

`docs/RESEARCH-AND-DESIGN.md` connects primary sources to actual implementation choices and names deferred features. `docs/VALIDATION.md` distinguishes checks run here from platform and listening checks still needed. Included WAVs are synthetic plucked tones, not recordings of instruments or hardware comparisons.

## License

AGPL-3.0-or-later, including corresponding source for redistribution. JUCE retains its own copyright and licensing terms; see `docs/JUCE-LICENSE.md` and `vendor/JUCE-8.0.12.tar.gz`. A proprietary release needs a separate licensing review. No third-party DSP code from the researched repositories was incorporated. Wolf Bloom source is reused for the Hall/Bloom foundation.
