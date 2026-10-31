# Studio Edition — native implementation and Linux validation delivered

Branch: `ui/deep-current-complete`. Native build **`6df7d3b58bbf`**, Tide **1.3.0**.
Continued from recovery `46b47a90147d882ec267f83b2493a014f3d93163` without reset;
preserved the shared-skin implementation and the correction-import commit.

[Implementation, validation, benchmarks, build instructions and exact limits](IMPLEMENTATION-VALIDATION.md)

[Main before/after](evidence/implementation/before-after-main.png) ·
[Browser before/after](evidence/implementation/before-after-browser.png) ·
[All 74 final / 70 baseline screenshots](evidence/implementation/SCREENSHOTS.md)

## Implemented

Shared Studio Edition skins across main, five modern machine pages, three legacy
expanded views, all three browser scopes, menus, selectors, editable values,
tooltips, callout shells, quick guide and plugin-owned alerts. Existing layout,
branding, controls, 25 algorithms, bindings, state and motion behavior are retained.
IBM Plex Sans functional typography and its SIL OFL are included.

The 29 recovered originals remain byte-identical. Four bounded/quoted Higgsfield
correction requests produced three completed source images and one failed attempt;
those originals are separately preserved. Seventeen reviewed runtime PNGs record
crops, masks, dimensions, hashes and slices. Fields reuse the accepted H07 inset
family; busy H08 sources and superseded arrow-bearing attempts are excluded. H12
is omitted. All decorative pixels are Higgsfield-derived; no material painting or
alternate generator was used. No additional image batch is needed to resume.

## Verified here

- Linux Release Standalone/VST3 build, all **12 CTest suites**, two recovery guard
  tests and Linux VST3 pluginval strictness 5, including editor automation: pass.
- **214 protected processing/parameter/preset hashes** and 14 editor-method
  invariants: pass. Five authorized visual/build hashes changed as expected;
  `PluginEditor.h` remains unchanged. The historical 220-file recovery freeze is
  preserved as historical evidence, not misreported as current UI immutability.
- Frozen baseline build `9c1e62f44667` versus candidate: **64-program audio fixture
  byte-identical**, SHA-256
  `10667ab972dc01517d67d20c9a52d37592bb79cee6177ca84d73713f9f8df728`.
- All 29 recovered and three correction originals, 17 exports, five runtime aliases
  and font/license hashes verify. Immutable decoded art: **21.98 MiB**; shared
  composed-skin cache: **8 MiB maximum**, existing rotary/backdrop caches retained.
- Normal and forced GUI benchmarks: three alternating trials at 1/4/8/16 open and
  never-opened editor cases, plus targeted repeats. Initial >10% timing increases
  did not reproduce in the targeted repeats. Peak open-process RSS increased about
  **31–39 MiB**. Both runs, CPU, timing, memory and limitations are published;
  no real-host or zero-dropout performance claim is made.

## Remaining / exact resume point

Generated panel highlights remain lighter/less saturated than the numerical palette
targets; native dark captions improve their readability without recoloring artwork.
Final color/contrast approval on a calibrated Mac display remains open. Some surfaces
upscale at the largest simulated 2× size; physical Retina sharpness is unverified.

Mac universal AU/VST3 builds, `auval`, Ableton scanning/use/automation/recall,
physical Retina, real-host performance/GPU/dropouts, keyboard/screen-reader review,
and installer install/uninstall/signing/notarization remain **pending**. Native
file pickers retain platform styling; manual browser edge-state interaction also
needs real-host review. Linux cannot produce or validate a Mac release package.

Resume on the latest branch head and run `bash scripts/Build-Mac.command` on a Mac
using the linked instructions. Do not regenerate the whole art family, reset the
branch or modify protected processing. If final material approval rejects a specific
asset, reconfirm current Higgsfield parameters/cost and correct only that asset.
