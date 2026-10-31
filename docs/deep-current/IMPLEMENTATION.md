# Deep Current refinement

Implementation branch: `ui/deep-current-refinement`.
Frozen baseline: `a54c5f143e01b0d3e7b56eb587881e2232248041`.
The processor, parameter and DSP files on that baseline are also byte-identical
to `development/tide-1.3` (`bbfbedda125c65357c5d07cc907ff0a46c598ce4`).

The uploaded detailed CODEX_MASTER_PROMPT.md supplies the visual brief. The user's
explicit requirement to preserve all 1.3.0 processing governs this change. This
branch changes rendering only; it retains the existing two-engine layout,
independent A/B machine views, presets, bindings, event handlers, display source
data, animation cadence and plugin identities.

## Implemented

- Cached blue-black satin chassis, graphite delay and teal reverb materials, dark
  glass, ivory text and a consistent shared system-sans palette.
- One stationary machined knob face from source 05, with cleaned alpha and repaired
  baked needle. Existing dynamic vector pointers/arcs and bounded size cache stay.
- Machined linear thumbs. Existing vector buttons, menus, selectors, focus and
  enabled/disabled treatments remain in the shared LookAndFeel.
- Shared colors on independent machine pages and all browser scopes. Branding
  remains original code-drawn text rather than generated mockup lettering.
- Expanded page identity is part of the backdrop cache key, preventing stale
  headings when changing between equal-size legacy pages.
- Five offline embedded 256px PNGs, asset-aware build identity, source archive
  resource inclusion and a reproducible Pillow preparation script.

## Asset decisions and limits

All 15 repository previews were checked for actual size, mode, alpha and file
readability, and reviewed in the supplied contact sheet. Material, knob and glass
sources were inspected individually. These are **1200px derivatives**, not the
2048/2688px originals advertised in the historical ASSET_MANIFEST.csv. Source 01
is JPEG; sources 02–14 are RGBA previews. The runtime crops come from 01, 02, 03,
05 and 08. The remaining references guide style only; invented controls and
lettering do not ship. No additional Higgsfield generation was needed.

See `assets/tide/deep-current/production/manifest.json` for exact crops, input
hashes, sizes and byte costs. All runtime textures are 256×256, shared and decoded
once on the GUI thread. Chassis/panels/glass are composed in the existing backdrop
cache; no new image decoding, file I/O, rendering, allocations or locks enter the
audio callback. The source needle repair uses adjacent brushed material and is
not claimed identical to a natively needle-free original.

## Preservation and verification

`python scripts/check-deep-current.py` checks 214 frozen source/preset hashes and
14 editor method hashes, including complete layout, constructor/bindings,
callbacks, timer, and both visualization methods. The only TideEditor method
changed is static backdrop painting. A targeted GUI regression check switches
between equal-size legacy expanded pages and compares heading pixels.

`tests/ui_refresh_checks.cpp --screens` captures main windows at 900, 1080, 1620
and 2×, independent pages, three preset-browser scopes, all 12 delay and 13 reverb
algorithm pages, and three legacy expanded views. Rendering screenshots does not
establish native mouse/keyboard, Retina, accessibility or DAW acceptance.

See `REFINEMENT-VALIDATION.md` and `evidence/refinement/` for checks actually run,
audio comparison, screenshots, measured Linux stress data and outstanding native
acceptance. Historical UI-REFRESH-1.0 evidence is retained as historical evidence,
not presented as measurements from this change.
