# Tide — Deep Current visual assets (UI-only)

This design-only branch is based on UI Refresh 1.0 commit `464e0c3f7047c2a50198a824b1cbaf61caed4bee`. It **must not** be merged into production or treated as a new audio build without regression testing.

## Start here
1. Read [CODEX_MASTER_PROMPT.md](CODEX_MASTER_PROMPT.md), [DESIGN_SYSTEM.md](DESIGN_SYSTEM.md), [PERFORMANCE.md](PERFORMANCE.md) and [QA_ACCEPTANCE.md](QA_ACCEPTANCE.md).
2. Review the actual Tide layout in [the existing UI Refresh screenshots](../test-logs/ui-refresh-1.0/after/main-1080.png). That functional layout takes priority over any generated mockup.
3. The images are listed in [ASSET_MANIFEST.csv](ASSET_MANIFEST.csv). An automated GitHub Actions import saves compact **design references** to `assets/tide/deep-current/previews/`, not release-ready runtime textures.
4. Use the independent [original 15-image source pack](https://d2ol7oe51mr4n9.cloudfront.net/user_3JKUae4GIiCUFu3kbupkoLE3mRV/c2f843ec-eab7-4925-8832-f86915a52e11.zip) only if full-resolution art is needed.
5. Run a source-and-UI audit before editing existing JUCE code.

## Hard constraints
Keep exactly the same two-panel layout, controls, algorithms, DSP, parameter identifiers/ranges/defaults, presets, plug-in identity, automation, routing, state persistence, and audio behavior. **No audio processor files should change.**

The design is original Wolf Audio. TimeLine and BigSky are functional inspirations only, never appearance templates.

This repository is public. All checked-in design references and documentation are public.

## Status
Visual source assets generated, **NOT implemented**. The previous UI Refresh 1.0 Linux tests applied only to that prior commit. New Deep Current needs fresh compilation, GUI tests, audio equivalence, native Mac AU/VST3 and Ableton testing, multi-instance graphics benchmarks, and release verification.
