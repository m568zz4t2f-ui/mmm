# Wolf Tide 1.3.0 — source build candidate

**Deep Current — Studio Edition:** a complete native visual pass using the recovered Higgsfield family, shared skins and embedded functional typography. See [screenshots, measured benchmarks, preservation evidence and remaining Mac checks](docs/deep-current-studio/IMPLEMENTATION-VALIDATION.md). All 214 protected processing/parameter/preset files remain unchanged; the 64-program Linux comparison is byte-identical. Final material/color approval and Mac release validation remain pending. [UI Refresh 1.0](docs/UI-REFRESH-1.0.md) is the historical baseline.

Integrated stereo delay/reverb plugin: twelve delay machines, thirteen reverbs, independent delay A/B and reverb A/B, Series/Parallel/Frequency split routing, advanced native controls and versioned project compatibility.

**Mac installer handoff:** unzip the self-contained Source ZIP and run `bash scripts/Build-Mac.command` on your Mac. It includes pinned JUCE source, builds universal AU/VST3/standalone, runs the tests and creates the package. Read [the Mac build guide](docs/BUILD-MAC-1.3.md) for prerequisites, the existing duplicate-install migration, signing, notarization and rollback.

The source is integrated and Linux-tested. This is a candidate for native build and musical acceptance, not a claim of a validated macOS release. No Mac binaries are supplied. Native AU/host/Installer.app validation and human listening acceptance remain required before distribution.

- [Quick Start](docs/QUICK-START.md)
- [All 25 algorithms, controls and routing](docs/ALGORITHMS-1.3.md)
- [Implementation and validation status](docs/TIDE-1.3-DEVELOPMENT.md)
- [Unified user brief](docs/TIDE-1.3-UNIFIED-BRIEF.md)
- [Release handoff and evidence](docs/RELEASE-1.3.0.md)
- [New portable presets](Factory-Presets/1.3): 64 combinations and 50 side starters. The 92 earlier files remain in their original locations for compatibility.

Existing sessions retain the legacy engine. The first 61 parameter IDs, order and normalized ranges remain unchanged; new parameters are appended. Each section has an explicit Legacy / Tide 1.3 processing choice. Factory/User/Modified identity, locks, A/B, undo-load, scoped user presets, search and favorites remain supported.

## Build and test

CMake 3.22+ and a C++17 compiler are required. Source ZIPs include the JUCE 8.0.12 archive; a Git checkout without it fetches pinned commit `29396c22c93392d6738e021b83196283d6e4d850`.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel 2
ctest --test-dir build -C Release --output-on-failure
```

Dependency-free DSP checks:

```sh
cmake -S . -B build-dsp -DTIDE_DSP_ONLY=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build-dsp --parallel 2
ctest --test-dir build-dsp --output-on-failure
```

The extended 1.3 checks are enabled by default (`TIDE_BUILD_13_LAB=ON`). On headless Linux use an X server/Xvfb for native UI/host tests. Linux development dependencies include ALSA, X11/Xext/Xinerama/Xrandr/Xcursor, FreeType/fontconfig and OpenGL. AU builds only on macOS. Windows is not validated.

The preset recipes and return trims have numerical and limited recording-based measurements. They are not a substitute for auditioning varied instruments in the target DAW. Prepared transition pools retain memory to keep allocations out of processing; see the measured memory/CPU budget in the handoff.

AGPL-3.0-or-later. Distribution must retain the license and corresponding source, including the pinned dependency sources. Historical release/audit documents remain under `docs`; use the 1.3 handoff for current status.
