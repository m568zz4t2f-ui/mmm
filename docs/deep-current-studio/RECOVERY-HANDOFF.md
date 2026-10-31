# Studio Edition artwork recovery

The authenticated Higgsfield history contains **30 Studio Edition jobs: 29 completed originals and one failed dropdown-glyph attempt**. These jobs were already present when this recovery began. No new generation was submitted, no additional generation credits were spent, and no alternative generator was used.

The checked-in preflight was stale: it correctly described the earlier implementation session but did not include these subsequent jobs. `recovered-higgsfield-jobs.json` preserves complete prompts, model/settings, reference lineage, original URLs, job IDs, dates and clearly labeled destination paths. The recovery workflow imports original bytes without changing them. Its receipt records measured dimensions, SHA-256, alpha bounds and any disagreement with service-reported dimensions. The workflow fails if any of the 214 protected sources/presets or six frozen visual/build sources differs from the freeze. Verify its success and imported files before assuming the images are on GitHub.

## Current decisions and evidence

- Production requests consistently use material master M02 (`b8504d01-4be3-41cb-b74f-2c3e574b3ff6`). Preserve this lineage. Master mockups remain material references; generated controls/text/geometry do not replace the real interface.
- Two normal-selector attempts (`51c1310e` and `bfebfd07`) are superseded by the blank corrected `11c9ba62` image. Retain them as provenance only.
- The final Reverb panel (`900fb647`) uses the actual graphite-panel generation as its first reference, establishing paired geometry lineage. Verify the output geometry rather than assuming the request succeeded exactly.
- The completed glyph (`26ab595d`) replaces the failed `d8ffe37f` attempt. No image exists for the failed job.
- H11 glyphs and H12 contours are optional. Preserve runtime semantics and omit decoration that competes with controls.

## Remaining implementation

1. Inspect full-resolution originals and contact sheets at actual plugin scale. The recovered files are **not automatically production-approved**. Confirm palette, lighting, paired geometry, pointer-free cap, quiet interiors and alpha quality. In the compact review, selector state interiors look more textured than button interiors; validate at final control size before acceptance.
2. Prepare deterministic crops/masks/size exports and slice margins. Preserve originals separately. Do not repaint/reconstruct decorative material or reuse the previous repaired knob as Studio Edition artwork. If a source requires invented replacement art, regenerate it through Higgsfield with a fresh cost preflight.
3. Record approved crops, actual dimensions, export hashes and runtime mappings in `asset-production-manifest.json`. Do not confuse the 29 originals with 29 finished runtime assets.
4. Integrate accepted artwork through the existing shared theme, cache and JUCE resources. Cover the main editor, all modern/legacy advanced views, browser scopes, selectors, menus, text editors, tooltips, quick guide and plugin-owned dialogs. Keep native text, exact pointers/arcs, focus and live visualizations. Keep the current control order, parameters, attachments and behavior.
5. Preserve protected source hashes. Run the existing tests, deterministic 64-program baseline audio comparison, control/recall/automation checks, all-view screenshots and before/after GUI measurements. Prior results establish the baseline only.
6. Complete Mac AU/VST3, Ableton, Retina and installer validation on a Mac. Do not label this a finished interface or validated Mac release before actual evidence exists.

Continue on `ui/deep-current-complete` from its latest head. Read `USER-VISUAL-PLAN.md`, `baseline-freeze.json`, the recovered job manifest and the measured receipt. **Do not restart generation or overwrite existing progress.** The intended sequence remains Higgsfield originals → verified GitHub assets → native implementation and tests.
