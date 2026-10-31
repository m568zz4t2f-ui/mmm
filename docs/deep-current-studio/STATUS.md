# Deep Current — Studio Edition: recovered artwork; implementation pending

Active branch: `ui/deep-current-complete`.
Frozen Tide 1.3.0 baseline: `e2a6ad644573569ba130d145403466cdd410c17f`, build `9c1e62f44667`.

## Verified recovery

- Recovered **30 existing Studio Edition Higgsfield jobs: 29 completed originals and one failed glyph attempt** from the authenticated generation history. No new job was submitted or generation credit spent during recovery.
- Preserved all 29 original PNGs, approximately 190.7 MiB total, under `assets/tide/deep-current-studio/originals/`, with clear H01–H12 and M01–M03 names. The original-import commit is `57eb3715dae76c53ee1b1f7fbb53f1e72310b314`.
- Preserved full submitted prompts, model/settings, dates, source URLs and reference lineage in `recovered-higgsfield-jobs.json`. Service-reported and measured dimensions agree for all 29 files.
- Recorded measured source SHA-256, dimensions, alpha bounds and original byte sizes in `recovered-artwork-receipt.json`. Two normal-selector attempts are explicitly superseded; the failed glyph has no image and is not counted among the originals.
- The recovery test and actual import/verify workflow passed. **All 220 frozen files remain byte-identical**: 214 protected processor/parameter/preset files plus six visual/build sources. No plugin code or audio DSP changed.
- Review contact sheets are under `assets/tide/deep-current-studio/review/`. These are review derivatives, not runtime atlases.

[Successful recovery workflow](https://github.com/m568zz4t2f-ui/mmm/actions/runs/41157744650)

## What remains unfinished

The earlier statement that Higgsfield tools are unavailable is historical. Higgsfield responded successfully in this recovery session; the existing artwork was found before any duplicate generations were started.

All 29 original PNGs are **fully opaque**, including their surrounding backgrounds. None is a validated runtime cutout. Crops, masks, exact runtime sizes, export hashes, state registration, slice margins and final visual approval remain pending. `asset-production-manifest.json` now points to the recovered candidates and retains `approved: false` until that work is verified. The M02 master establishes production-request lineage; mockup labels, geometry and controls are not authoritative.

Continue native implementation using only accepted prepared artwork, preserving real control bounds and functions. Cover the main editor, all modern and legacy advanced pages, browser scopes, popups, text editors, tooltips and plugin-owned dialogs. Retain live text, exact pointers/arcs, focus indicators and existing visualizations. Follow USER-VISUAL-PLAN.md and RECOVERY-HANDOFF.md. If another corrective generation is actually necessary, use Higgsfield after confirming its current quote; do not duplicate the recovered family or substitute another generator.

No new plugin binary was built in this recovery. New 64-program audio comparisons, behavior/automation/recall tests, whole-view screenshots, before/after GUI measurements and Mac AU/VST3/Ableton/Retina/installer validation remain implementation/release work. Prior test logs remain baseline evidence only. This is not a completed Studio Edition or a new validated Mac release.
