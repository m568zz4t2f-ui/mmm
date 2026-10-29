# Codex master implementation handoff — Wolf Audio Tide Deep Current

You are working in public repository `m568zz4t2f-ui/mmm`, design branch `design/tide-deep-current`, starting from UI Refresh 1.0 commit `464e0c3f7047c2a50198a824b1cbaf61caed4bee`. The purpose is a visual-only refinement of existing Tide 1.3.0 JUCE AU/VST3 source.

## First actions
1. Verify GitHub repository/branch and latest head; confirm an actual usable shell and build environment.
2. Read this file and every file in `docs/deep-current/`, including the manifest, palette, performance and QA specifications.
3. Inventory current `Source/PluginEditor.*`, `Source/TideLook.h`, shared advanced panel, preset browser, resources, scripts, and all existing parameter attachments. Confirm source and actual install version.
4. Preserve source baseline and hashes; use a new implementation branch based on this design branch. Do not merge to main automatically.
5. Use `assets/tide/deep-current/previews/` after design-asset import for compact reference images; use original manifest URLs/ZIP for full-resolution art where needed. Check actual image alpha, geometry, colors, scaling, visual artifacts; all AI art is provisional.

## Scope: visual only
- Do not modify DSP/audio algorithms, audio thread, processing order, routing behavior, timing, core state or presets.
- Do not change manufacturer/product/bundle IDs, parameter identities/ranges/defaults, automation, host I/O, preset storage, program order, saved projects, signal path or host compatibility.
- Keep Delay left gray and Reverb right turquoise and all existing layout/hit targets, knobs, selectors, advanced controls and master footer. Actual working editor and screenshots take priority over conceptual reference.
- TimeLine / BigSky are *functional inspiration* only; visual appearance must be original Wolf Audio.

## Implement
A. Prepare curated runtime assets from original source artwork 01–14. Crop, alpha-clean, resize for 1x/2x, reject mismatched or poor geometry, optimize memory, ship fonts only when licensed. Keep source art outside plugin runtime.
B. Improve shared LookAndFeel across main, advanced and preset browser: coherent modern readable sans, consistent weights/sizes/spacing, tabular values, all UI control states.
C. Chassis and two panel backgrounds: static cached warm graphite / dark teal satin metal, restrained navy outer chassis, clean lighting. No rearrangement.
D. Main + secondary custom knobs: unified smoked titanium face, code-drawn correct pointer/arc/readout, avoid rotating baked lighting. Preserve interaction.
E. Delay and Reverb displays: high-quality inset glass bezel, preserve stereo delay tap visualization and current flowing reverb geometry. Optimize existing motion, do not add new audio analysis.
F. Modern matching buttons, selectors, slider caps, status glyphs and original micro branding. Prefer scalable native JUCE vector/nine-slice surfaces rather than raw AI contact sheets.
G. Apply to every relevant editor/dialog/menu/preset view; no hidden functional controls.

## Performance
Graphics are on GUI thread only. Static art decoded once and reused. Existing visual data and timer strategy must be profiled, not arbitrarily replaced. Test 1/4/8/16 editor instances, closed/open, on native Mac in addition to existing Linux stress harness. Optimize any meaningful CPU/GPU/memory regression. Keep motion appealing and purposeful.

## Acceptance/delivery
- Prior tests on UI Refresh 1.0 are historical only; run fresh CTest, strict VST3 GUI/pluginval, audio byte comparison if reproducible, protected-source/preset checks.
- Test 900/1080/1620 + Retina and every advanced editing page.
- Perform Mac AU/VST3/universal/Ableton checks when environment permits; document *pending* otherwise.
- Provide source commit(s), changed files, before/after screenshots, optimized resource manifest, CPU profiling and exact evidence; no invented pass results.
- Never auto-install, overwrite system plugins, sign or notarize without prerequisites/authorization.
- Do not undertake unrelated DSP fixes. Flag separately.

## Existing references
- Original approved UI: `docs/test-logs/ui-refresh-1.0/after/main-1080.png`.
- Prior benchmark and source safeguards: `docs/UI-REFRESH-1.0.md`.
- New moodboard: `Tide_00` from manifest, not a layout template.
- Original user-supplied screenshot and conceptual mockup live in the earlier chat handoff archive. Source UI always overrides mockup labels/positions.

Implement as an iterative, reviewable visual-only change, not an untested whole-plugin rewrite.
