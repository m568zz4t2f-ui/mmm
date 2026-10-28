# Tide UI Refresh 1.0 — source candidate

This is the coordinated visual update to the integrated Wolf Tide 1.3.0 candidate. It keeps the two-panel gray Delay / turquoise Reverb layout and the existing workflow. The plugin release remains 1.3.0; UI Refresh 1.0 identifies this design revision. The compiled source-build label distinguishes it from the earlier 1.3 source candidate.

## What changed

One shared JUCE look-and-feel now supplies the main editor, independent-machine advanced pages and preset browser. A single system sans family, coordinated type sizes, restrained borders, dark controls, warm gray metal and teal surfaces replace the previous mixed styling. Custom rotary faces use original metallic artwork, etched needles, thin side-colored value arcs and existing precise numeric text entry. Keyboard focus, hover and disabled states remain visible. Native file pickers retain their platform behavior; plugin help/error messages and preset menus use the shared theme.

Delay graphics show stereo lanes, timing/grid, feedback decay and the four modern Multitap timing/level controls. The reverb display retains flowing waves with depth and shimmer detail. These displays illustrate the primary engines; they are not oscilloscopes or a complete visualization of all four signal paths.

Static panel textures and the knob base are cached. A bounded eight-size knob-image cache avoids repeated resampling at physical display sizes. Nine preallocated wave paths reuse spatial sine/cosine samples. Parameter changes repaint the display regions; hidden main and advanced editors suspend their timers. No graphical work was added to the processor/audio thread. Active motion retains the original 24 Hz cadence after testing a more expensive 30 Hz candidate; idle updates remain 12 Hz.

## Compatibility and sound

Baseline source: `bbfbedda125c65357c5d07cc907ff0a46c598ce4`. The eight processor, parameter and DSP files listed in [the protected-source manifest](test-logs/ui-refresh-1.0/protected-source.json) remain byte-identical. All 165 parameter identities/ranges/defaults, routing, preset/state formats, program order, four-slot capabilities and plugin manufacturer/product/bundle codes are preserved. All 206 portable preset files remain unchanged.

The included `tide_ui_refresh_checks --audio` fixture renders all 64 programs with burst/tail audio at 48 kHz and 256-sample blocks, using the same compiler/toolchain and a hidden editor. Baseline and refresh each produce 12,582,912 bytes, byte-identical with SHA-256 `10667ab972dc01517d67d20c9a52d37592bb79cee6177ca84d73713f9f8df728`. This is a targeted deterministic comparison; unchanged DSP source is the stronger scope guarantee. It does not replace musical listening or establish every host's floating-point behavior.

## Rendering comparison

Old → refresh; medians of three trials, all times in milliseconds. Final source build `327e76352705`.

| Instances | Editors | Process CPU | Audio batch mean | Full paint | Event lag maximum |
| --- | --- | --- | --- | --- | --- |
| 1 | closed | 69.50 → 66.67 | 0.25 → 0.22 | 0.00 → 0.00 | 4.25 → 0.75 |
| 1 | open | 634.01 → 659.15 | 0.30 → 0.31 | 10.86 → 12.45 | 8.38 → 9.82 |
| 4 | closed | 218.46 → 284.87 | 0.87 → 1.14 | 0.00 → 0.00 | 1.97 → 3.78 |
| 4 | open | 1507.40 → 1562.51 | 1.08 → 1.08 | 43.52 → 49.38 | 45.45 → 68.26 |
| 8 | closed | 622.71 → 610.87 | 2.41 → 2.52 | 0.00 → 0.00 | 88.58 → 22.53 |
| 8 | open | 1857.01 → 2012.53 | 2.44 → 3.08 | 99.45 → 110.67 | 118.66 → 141.20 |
| 16 | closed | 929.74 → 1183.02 | 3.91 → 4.99 | 0.00 → 0.00 | 0.66 → 37.08 |
| 16 | open | 2426.98 → 2366.43 | 4.25 → 4.09 | 240.90 → 170.30 | 310.98 → 234.75 |

The final run also shows scheduling noise in closed-editor audio processing, despite identical processor/DSP source (for example the refresh 16-closed third trial took 2.04 s rather than the intended 1.25 s). Treat both faster and slower figures as stress observations, not DSP speed changes. A final native acceptance decision remains open.

The fixture creates 1, 4, 8 or 16 processors, processes 235 blocks at 48 kHz / 256 samples, and measures actual processing time per batch across all instances. Open trials create native editor windows with MOTION enabled and also force complete software paints into preallocated 1080×720 images at a requested 30 Hz; native dirty-region timers run concurrently. Closed trials create no editor. This deliberately stresses rendering twice, so the repaint figures are not ordinary DAW paint costs. Process CPU is summed CPU milliseconds across threads, not a percentage; CPU / elapsed gives equivalent fully occupied cores. Event lag is the maximum observed timer lateness in each trial.

Three trials per cell were interleaved between frozen baseline and refreshed binaries, with alternating order, on the same Linux/Xvfb cloud runner. Medians are reported; raw CSVs preserve every maximum and elapsed time. The shared runner is noisy, particularly at 16 windows. Both interfaces exceed the 33 ms forced full-paint budget at four or more windows. These figures cannot certify low-latency audio deadlines or smooth sixteen-window use. Native Mac/DAW profiling remains an acceptance gate. The 30 Hz candidate CSVs are retained to explain the animation decision, not as final results.

To reproduce after a Release build:

```sh
build/tide_ui_refresh_checks --screens /absolute/path/screens
build/tide_ui_refresh_checks --audio /absolute/path/audio.raw
python3 tests/ui_refresh_bench.py build/tide_ui_refresh_checks refresh.csv \
  --baseline /absolute/path/frozen-baseline-checks --baseline-output baseline.csv
```

On Linux set DISPLAY to an X server (Xvfb is sufficient). The script registers normal window-manager atoms on bare Xvfb and keeps their connection alive. For a comparable baseline, build the same helper against the baseline source, in a separate checkout with the same toolchain/JUCE/options, before applying UI changes. Do not run other CPU-heavy jobs during profiling. Repeat on the actual Mac with your DAW and display scales.

## Validation and acceptance

Release build `327e76352705` passed all 11 CTest suites (48.27 s) and pluginval 1.0.4 strictness 5 with GUI/editor automation enabled, seed `0x1301`, requested sample rates 44.1/48/96/192 kHz. The suites cover state migration/recall, parameter controls, all integrated algorithm paths, preset identities, scoped loads, locks, A/B and UI/advanced automation. Logs: [CTest](test-logs/ui-refresh-1.0/ctest.log), [pluginval](test-logs/ui-refresh-1.0/pluginval.log). The audio comparison passed after this final build.

[Before](test-logs/ui-refresh-1.0/before/main-1080.png) · [After](test-logs/ui-refresh-1.0/after/main-1080.png) · [Advanced](test-logs/ui-refresh-1.0/after/advanced-0.png) · [Browser](test-logs/ui-refresh-1.0/after/presets.png)

Screenshots include 900, 1080 and 1620 widths, a 2× snapshot, all five independent-machine pages and the preset browser. These check geometry and antialiasing under Linux software rendering; they are not evidence of native Retina/DAW rendering. The source ZIP includes the original brief, before/after captures, raw measurements and checks.

| Requested acceptance item | Evidence / status |
| --- | --- |
| Coordinated typography | Shared family and theme applied; Linux screenshots reviewed |
| Sharp custom knobs and accurate values | Scale snapshots; existing parameter text/bindings retained |
| Recognizable colors and layout | Gray/teal two-panel structure preserved |
| Unified controls, menus and dialogs | Shared plugin styling; OS file pickers remain native |
| Smooth expressive displays | Cached paths, static/activity/motion retained; native motion review pending |
| Sharp supported display scales | Linux 900/1080/1620 and 2× snapshots; Mac scales pending |
| Preset and automation regression | Automated host/UI suites; native DAW recall pending |
| Unchanged sound/DSP | Eight protected files identical; all-program audio fixture identical |
| Acceptable CPU/rendering | Cloud comparison recorded; native acceptance pending, especially many windows |
| Builds and loads in supported DAWs | Linux build/pluginval; Mac AU, target DAWs and installer pending |

No claim is made that all ten release acceptance items are complete across supported platforms. This delivery is ready for the native Mac build and acceptance stage.

## Build the Mac installer

Extract the ZIP into a clean directory, open Terminal in `WolfTide`, and run:

```sh
bash scripts/Build-Mac.command
```

The ZIP contains pinned JUCE 8.0.12 source, AU/VST3/standalone build/package scripts, tests, all factory presets, installation-conflict detection, backup/rollback tooling and licensing. See [the Mac guide](BUILD-MAC-1.3.md) for prerequisites and exact validation/signing steps. The default local package is `dist/Wolf-Tide-1.3.0-unsigned.pkg`. No Mac binary or installer was produced here. Save the compiled build ID when comparing installations with the earlier 1.3 candidate. Main and the live Mac installation remain unchanged.
