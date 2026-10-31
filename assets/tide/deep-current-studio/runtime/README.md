# Studio Edition runtime artwork

All decorative pixels come from the recovered Higgsfield family or the three
quoted Higgsfield material corrections. No stock art, alternate generator,
repainting, recoloring or reconstructed knob pointer is used.

`python scripts/prepare-studio-artwork.py` reproduces the 17 PNGs using Pillow.
`manifest.json` records original job/model, hashes, crop rectangles, masks, export
sizes, source slice margins (left/top/right/bottom), logical corners and controls.
Originals remain unchanged in `../originals/`. Corrected originals have their own
receipt; the historical recovery receipt remains intact.

A deterministic exterior flood mask removes only dark exterior pixels connected
to crop corners. Enclosed glass stays opaque. The pointer-free cap is cropped and
resampled; no repair painting is required. The same fixed-lit cap serves all rotary
sizes. The chassis is reused for expanded editors through nine-slice mapping.

The decoded immutable PNG budget is 21.98 MiB. Small composed skins share a bounded
96-entry / 8-MiB GUI-thread cache, with oversized physical scales bypassing it.
Rotary caps retain the existing eight-size cache; per-editor backdrops retain their
existing resize/page cache. CMake fingerprints the exact PNG and embedded font bytes.

Native text, semantic ticks/arrows, focus/pressed/latched indicators, value arcs and
pointers, output indication and both state-fed visualizations remain runtime drawing.
No rendering/decoding/file access is added to the audio callback. The old Deep Current
runtime resources remain in history/source for comparison and are not linked here.

Selector and text fields reuse the approved shallow-inset H07 button family,
with native arrows and pressed/focus borders. The recovered H08 interiors are
too textured at native size; none are linked. The two arrow-bearing superseded
selector attempts remain excluded. Alias mappings are explicit in both manifests. H12's optional decorative
waves are omitted to keep them distinct from the real state-driven displays.

IBM Plex Sans regular/medium is embedded for consistent functional typography;
its SIL OFL and source hashes are in `../fonts/`. Original italic Tide branding
keeps its prior system-face treatment. Warm ivory text is slightly brightened for
readability; panel headings, Character and Combined Preset captions use dark ink on the
brighter metal highlights. Generated broad highlights remain lighter and less
saturated than the numerical palette targets; final art approval on a calibrated
Mac display remains pending. The source materials have not been recolored. Original
labels, hit regions, units, precision, input handling and parameter bindings remain.
