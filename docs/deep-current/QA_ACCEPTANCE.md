# Tide Deep Current — validation and acceptance checklist

## Protected baseline
- Branch starts from `464e0c3f7047c2a50198a824b1cbaf61caed4bee` UI Refresh 1.0.
- Source release family 1.3.0; build ID of that prior delivery: `327e76352705`.
- Prior report: 165 parameter identities/ranges/defaults, 206 portable presets, 64 deterministic program render comparisons byte-identical, 11 CTest suites and Linux VST3/pluginval passed **for the old build only**.
- Establish a protected-source hash manifest *before* UI changes. Ensure all processor, DSP, parameter mapping and plugin identity sources remain unchanged.

## Visual
- Main two-panel layout and all control coordinates/hit areas remain recognizable and usable.
- Knob sizes/angles/arcs/values accurate, no clipping, no image rotation artifacts.
- Typography consistent and legible at 900/1080/1620 and 2x.
- Delay taps and reverb lines remain informative and smooth; active/disabled/locked states distinguishable.
- All advanced Delay/Reverb pages, preset browser, routing/global controls, popups, help and error messages receive the same theme.

## Behavior
- All effects, 4-slot routing, separate and combined presets, preset load/undo, A/B, lock, hold, Mix, Bloom, Send, Sync, Clear Tails, automation, modified state, DAW project recall unchanged.
- Verify AU/VST3 editor creation, opening/closing, resizing, DPI, focus and message thread safety.
- Re-run CTest and Linux VST3 validation, including UI automation.
- Compare audio render across all 64 programs against preserved baseline and confirm unchanged parameter IDs/state formats/presets.

## Native Mac release blockers
- MAC-01/02: build Mac and universal Intel/Apple Silicon runtime.
- AU-01: auval and actual installed binary/build identity.
- DAW-01: Ableton AU/VST3 load, automation, state recall, screen scaling, graphics.
- PKG-01/02: unsigned package installation, conflict migration and rollback.
- SIGN-01: Developer ID signing and notarization when credentials exist.
- AUDIO-01: human listening for broader 1.3 release.
- PERF-01/02: native multi-instance render/CPU acceptance; previous forced full paints were >33ms at >=4 windows.
- INSTALL-01: check duplicate old 1.2.x installs and paths on actual Mac.

## Reporting
Include before/after main/editor/browser screenshots, hashes/modified-file list, test logs, benchmarks with environment, specific outstanding issues. Do NOT claim a Mac build or installer exists until proven.
