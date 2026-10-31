# Wolf Audio Tide 1.3.0 — Higgsfield Visual Update Plan

Prepared October 30, 2026. Planning deliverable only; no generations submitted, plugin code changed, or new performance results produced.

## 1. Design decision

Develop **Deep Current — Studio Edition**: an original, restrained studio interface with graphite Delay, deep teal Reverb, smoked titanium controls, dark glass displays, and warm ivory typography. Keep Tide’s recognizable two-engine layout and expressive displays. The result should feel like one professionally manufactured instrument, including every popup and advanced page.

Use Higgsfield for **all newly created visual artwork**. Do not mix in stock graphics, another image-generation service, unrelated icon packs, or code-painted replacement materials. Existing Wolf Audio/Tide branding remains an identity reference, not an invitation to invent a different logo.

Functional labels, editable values, parameter arcs/pointers, focus indicators, and live visualizations remain runtime UI drawing. These are necessary representations of the plugin’s state. Higgsfield supplies their visual direction and all decorative surfaces. A literal interface made entirely of generated pixels would prevent reliable dynamic text, accurate controls, accessibility, and responsive visualizations. This plan explicitly uses Higgsfield-only artwork with native functional rendering.

## 2. What was actually reviewed

Repository: `m568zz4t2f-ui/mmm`. Inspected design branch: `design/tide-deep-current`, commit `e1aea64e373a537584d6c60222fc63aa7d52859d`.

- `CMakeLists.txt` declares WolfTide 1.3.0 and JUCE 8.0.12.
- Read `PluginEditor.cpp`, `TideLook.h`, `ModernPanel.h`, and `PresetBrowser.h` for rendering, layout, controls, and hidden views.
- Visually inspected the checked-in main 1080 screenshot, preset browser, independent-machine routing/global view, unified generated concept, and selected chassis/secondary-knob/button/icon art.
- Read the Deep Current design system, asset manifest, README, performance requirements, QA checklist, prior UI Refresh report, and the Library master handoff.
- The design branch documents the earlier UI Refresh baseline as `464e0c3f7047c2a50198a824b1cbaf61caed4bee`. Its screenshot shows build `327e76352705`.

This is repository evidence. The currently installed Mac plugin was not inspected or run. The other source artworks still need a full-resolution production audit; sampled compact previews do not establish production readiness.

### Findings that shape the new plan

| Observed condition | Required response |
|---|---|
| The working UI is 1080 × 720 with large stereo Delay/Reverb displays and four primary knobs per side | Generate for those real bounds, preserving screen prominence and control order |
| The generated concept changes layout, names, controls, and branding | Treat it as a material reference; never implement its invented interface |
| The inspected secondary knob has its indicator baked into its face | Regenerate a pointer-free cap; retain the accurate runtime pointer |
| The inspected button contact sheet depicts knob-like objects in rounded squares | Generate actual rectangular button skins matching Tide’s control family |
| Icon sheet includes symbols and lettered artwork unrelated to exact current controls | Keep existing text semantics; only produce icons for verified existing uses |
| Main, modern machine pages, legacy expanded panels, preset scopes, menus, and dialogs are separate surfaces | Include them all in the same design pass |
| Current source already caches backgrounds/knob sizes and retains 24 Hz active / 12 Hz idle display cadence | Reuse this approach; measure the effect of new assets before changing motion |

## 3. Locked visual system

### Materials and palette

| Element | Target | Treatment |
|---|---|---|
| Outer chassis | `#19272E` | Satin blue-black titanium; very low contrast texture |
| Delay panel | `#5B6568` | Warm graphite satin metal; neither beige nor bright silver |
| Reverb panel | `#246E76` | Deep desaturated teal anodized metal |
| Screen well | `#0C222A` | Dark, almost matte glass; restrained edge reflection |
| Main text | `#F4EADC` | Warm ivory; clean, opaque, highly readable |
| Delay active accent | `#D6CBB8` | Muted champagne, used on arcs and selected states |
| Reverb active accent | `#77D7D5` | Soft aqua, used on arcs and selected states |
| Secondary text | `#B9C7C8` | Use on dark surfaces after contrast checking; use ivory on lighter graphite |
| Warning | `#FFBC7A` | Existing warning semantics, reserved for real warnings |

These are art-direction and runtime targets, not colors a generator can be assumed to reproduce exactly. Measure approved output and reject material drift; no independent per-asset color reinterpretation.

One broad light source from upper left. Every bevel, reflection, and shadow must agree with it. Short soft shadows, narrow rims, shallow recesses, fine restrained grain. Texture should disappear behind text at normal viewing size. No screws, chrome glare, heavy industrial wear, decorative meters, neon clouds, embossed labels, or new ornamental controls.

Use a small consistent corner-radius family: approximately 12 logical pixels for large panels, 8 for screens, 5 for fields/buttons, subject to preserving actual bounds. Avoid stacking several heavy borders around every element.

### Typography and identity

Preserve the existing italic **Tide** identity and Wolf Audio name. Do not replace them with the generated concept’s widely spaced TIDE or wave logo. If a production brand image is required, use the existing brand reference through Higgsfield and reject any changed lettering or silhouette. Keep the existing wordmark if reproduction is imperfect.

Use one font family across all functional surfaces. Default to the existing system sans to minimize dependencies; adopt one redistributable embedded family only if cross-platform visual review justifies it and the license is included. Do not ask a generator to invent a font.

At the 1080 reference width: panel headings about 18 px; functional fields and values 13–14 px; button text around 11.5–12 px; parameter captions around 11–12 px. Preserve label wording, units, capitalization, and numerical precision. Verify actual 900-width legibility before accepting smaller captions. Use tabular numerals if supported by the selected face.

Use runtime text throughout. No generated labels or values in runtime artwork. Check normal text against a 4.5:1 design target and ensure focus/active states are identifiable by shape or border as well as color. Disabled information remains readable.

## 4. Real layout and view coverage

Keep the left Delay / right Reverb / bottom master hierarchy and current hit regions. At 1080 × 720, the inspected source uses:

| Region | Reference bounds x, y, w, h | Visual update |
|---|---|---|
| Delay panel | 24, 105, 510, 465 | Graphite material with restrained contour detail |
| Reverb panel | 546, 105, 510, 465 | Matching teal material and edge treatment |
| Delay display | 42, 233, 474, 145 | Dark glass housing; retain stereo lanes and timing dots |
| Reverb display | 564, 233, 474, 145 | Same housing family; retain flowing reverb lines |
| Combined/master strip | 24, 585, 1032, 105 | Dark titanium, consistent selectors, buttons, Mix/Bloom controls |
| Main knob labels/controls | Four per panel, starting near y=390 | One shared cap, matching size and accurate values |

Preserve Delay’s Time / Feedback / Tone / Amount and Reverb’s Decay / Pre-delay / Tone / Amount. Preserve engine presets, Character, Sync/subdivision, Ready/On/Tail/Off, Hold, Lock, Edit, processing revision selectors, Clear Tails, A/B, Undo Load, Activity modes, Browse, Routing, Send, Lock Mix, status, tempo, and existing meters.

The source contains 12 delay choices and 13 reverb choices. Artwork must accommodate real names such as Pattern / Multitap, Granular Pitch, Cloud Ensemble, Halo Shimmer, and Magnetic Drum. Do not reduce these to four convenient decorative tabs.

Apply the complete theme to:

1. Main editor at 900, 1080, 1620 widths and physical 2× scale.
2. Modern Delay A, Delay B, Reverb A, Reverb B, and Routing/global callouts, including scrolling and algorithm-specific controls.
3. Legacy expanded Delay, Reverb, and Routing panels; the expanded reference height is 1130.
4. Combined, Delay, and Reverb preset browser scopes: search, filters, favorites, recent, rows, save/load, import/export, empty/error states.
5. Selector popups, parameter value editors, tooltips, quick guide, warnings, and plugin-owned dialogs.

Native operating-system file pickers retain platform styling. Do not replace them to force decorative consistency. No new navigation, audio features, or moved controls are needed for this update.

## 5. Higgsfield production method

### One reference chain

Use the actual Tide screenshot for geometry, the existing brand for identity, and one selected master rendering for materials. They have distinct roles: the geometry reference overrides stylistic inventions.

The current Higgsfield catalog recommends **FLUX 3 Image** (`flux_3_image`) for multi-reference generation/editing and lists up to ten image references with 2K/4K classes. Use it as the first pilot candidate, then lock one model and one accepted material reference for the production family. Catalog support is not proof of usable asset precision. The detailed model and cost calls failed during this review; reconfirm parameters, availability, and pricing before execution.

Do not silently mix models to repair isolated pieces. If a different model is necessary, run a new consistency pilot and review the affected family together. Keep the same prompt prefix, reference set order, lighting, camera, palette, and approved export rules. Seeds are not an assumed capability.

### Stages and gates

**A — Geometry and inventory.** Capture current native main/advanced/browser screens when the Mac is available. Export a control/view inventory and exact bounds from the selected source. Freeze the working baseline. Use a dedicated implementation branch only when implementation starts.

**B — Material master.** Generate three controlled candidates for the same layout and palette, varying only metal roughness and depth. Select one cohesive master. Inspect at actual plugin size, not only as a large glamour image. No production asset batch until the master passes.

**C — Component pilot.** Generate one pointer-free knob cap, one real rectangular button state family, one display bezel, and one selector skin against that master. Composite into the real screenshot for internal review. Reject inconsistent lighting, geometry, cramped text zones, baked pointers, or material mismatch before generating the rest.

**D — Production assets.** Generate each required surface independently against the accepted references. Prefer one isolated asset per image. Contact sheets are review aids; they are not automatically runtime atlases. For states, use edits of the approved base with explicit unchanged geometry.

**E — Preparation and integration.** Deterministic cropping, alpha masking, cleanup, resizing, and atlas packing may prepare the generated artwork; these steps do not invent new decorative art. If a usable asset requires repainting or redrawing, regenerate it in Higgsfield. Keep source generations separate from compact runtime exports.

**F — Whole-product review.** Review every view together at real size, then run behavior, sound-equivalence, host, and performance checks. Fix family-level inconsistency at its shared source rather than decorating each window separately.

## 6. Production asset manifest

Sizes below are logical targets and maximum required physical coverage, not promised generator output sizes. Higgsfield may return different pixel dimensions. Preserve aspect ratio, inspect usable detail, and export exact runtime sizes afterward. A 1620-wide editor on a 2× display can need up to 3× the 1080 reference detail.

| ID | Asset family | Production specification |
|---|---|---|
| H01 | Chassis material | Clean 3:2-compatible source; enough real detail for 3240 × 2160 main coverage; generate a compatible taller surface for expanded layouts if needed |
| H02 | Delay panel | 510 × 465 target; 1530 × 1395 maximum coverage; blank and graphite |
| H03 | Reverb panel | Exact counterpart to H02, deep teal; same bevel, grain scale, light, radius |
| H04 | Master strip | 1032 × 105 target; 3096 × 315 coverage; text-free with quiet control areas |
| H05 | Universal knob cap | Front-on circular cap, no pointer, no value arc, no tick marks; approximately 384 × 384 clean master is sufficient for current main sizes |
| H06 | Screen bezel/glass | 474 × 145 target; 1422 × 435 coverage; one shared family, no waveform or text; optional highlight kept very faint |
| H07 | Button skins | Rectangular 26–34 px-high family; normal/hover/pressed/active/disabled plus focus treatment; slice-safe center and fixed cap geometry |
| H08 | Selector/text field skins | Same inset construction as H07, widths supporting 180–376 px fields and wider modern controls; blank interiors |
| H09 | Linear control skins | Quiet track and isolated thumb; position/fill/value remain live; same style in main and advanced views |
| H10 | Callout/browser surfaces | Dark titanium body and compatible list/row surfaces; clean stretchable regions; no generated text |
| H11 | Required utility glyphs | Only verified existing semantic uses; separate, high-contrast, matched stroke weight; no invented toolbar |
| H12 | Optional brand contour | One restrained current-line motif reused sparingly; omit if it competes with controls |

Use the same knob cap at primary and secondary sizes unless a real small-size legibility problem requires a companion. Its lighting stays fixed; only the live indicator moves. Do not rotate a lit cap bitmap. Do not independently generate a 100-frame knob strip.

For UI state skins, use tight paired exports with identical outer bounds. Hover is a restrained rim change; pressed is a shallow inset; latched is a persistent accent border/fill; keyboard focus is distinct; disabled is reduced but legible. Lock, Hold, Send, engine status, warnings, and preset modification retain their real meanings. Keep text labels wherever icons would be ambiguous.

Record each asset’s Higgsfield job/model, prompt, reference identifiers, generation date, actual pixel dimensions, source/output hashes, alpha bounds, crop, logical size, runtime export sizes, slice margins, and intended controls. Preserve the original generation unmodified.

## 7. Reusable Higgsfield prompt set

### Master prefix — repeat unchanged

> Wolf Audio Tide professional stereo delay and reverb audio plugin. Original Deep Current Studio Edition identity. Orthographic front elevation, no perspective distortion. Satin blue-black titanium chassis, warm graphite Delay panel, deep desaturated teal anodized Reverb panel, smoked titanium controls, dark glass wells, warm ivory typography reference. One broad soft light from upper left, consistent short soft shadows and thin bevels. Restrained fine grain, calm negative space, clear instrument hierarchy. Match the supplied approved material reference exactly. The actual Tide UI screenshot controls layout and component count. No invented controls, extra meters, screws, chrome glare, neon glow, ornamental machinery, or borrowed pedal branding.

### Material master addition

> Restyle the supplied real Tide screen while keeping every control, panel, screen, and master-strip position. Preserve the large stereo Delay and Reverb visualization areas. Improve material refinement and consistency only. Keep the original italic Tide identity. Any sample text is a placement reference; it will be drawn as live text in production. Do not replace dropdowns with tabs or remove the lower control strip.

### Knob addition

> Render one isolated pointer-free smoked titanium knob cap, perfectly circular, directly facing the camera, centered with generous consistent padding. Match the approved reference material and upper-left light. No indicator, marking, letters, numbers, arc, scale, background panel, or second object. Keep reflections restrained enough that a live ivory pointer will remain readable. Return clean separation from the background; use true transparency only when supported and verify the result.

### Panel addition

> Render only the blank [graphite Delay / teal Reverb] panel surface at the supplied target aspect ratio. Match its paired panel’s bevel width, light, grain scale, depth, and radius. No holes, labels, controls, icons, screens, logos, or painted shadows suggesting other components. Quiet flat interior suitable for readable overlaid controls.

### Button/selector state addition

> Render one isolated blank [rectangular button / selector field] matching the provided approved base. State: [normal / hover / pressed / latched / disabled]. Preserve identical camera, outer dimensions, corner shape, bevel, and material placement. Change only the specified state treatment. No knobs, text, arrows, symbols, or decorative panel. Keep the center visually quiet and suitable for horizontal slicing.

### Display addition

> Render an empty recessed dark-glass display bezel matching the approved material reference, at a 474:145 target ratio. Front-on, shallow inset, thin satin metal rim, subtle fixed upper-left glass reflection. No waveform, bars, text, numbers, LEDs, or reflections across the main information region. Generate the housing only; real stereo taps and flowing lines will remain live.

Use these with confirmed references and supported aspect ratios. A prompt request is not evidence that transparency, precise geometry, or exact lettering was delivered.

## 8. Runtime and motion plan

Replace the current procedural background/knob artwork with prepared Higgsfield resources. Reuse the existing shared theme, size cache, component attachments, value formatting, event handling, display curves, and invalidation behavior. Centralize approved tokens and art mapping in the existing theme; avoid a new rendering engine or plugin UI framework.

The primary UI files are `Source/PluginEditor.cpp`, `Source/TideLook.h`, `Source/ModernPanel.h`, `Source/PresetBrowser.h`, and possibly the editor header. A resource declaration in CMake will be necessary; preserve plugin identity and build behavior. If resource changes are not covered by the current source-derived build fingerprint, include them in its identity calculation so different artwork builds remain distinguishable.

Cache static panels/bezel/surfaces. Decode resources once and reuse immutable images across instances where practical. Use verified slice margins on flexible skins; do not stretch texture grain or bevels arbitrarily. Keep readable text and accurate interaction above the skin.

Retain stereo delay taps, the existing reverb curves, and STATIC / ACTIVITY / MOTION modes. These are illustrative primary-engine displays, not measured four-slot oscilloscopes. No generated video inside the plugin, fake moving waveforms, new FFT, or additional audio analysis. Preserve attractive motion and optimize it through profiling.

Use the existing 24 Hz active / 12 Hz idle schedule as the starting baseline. Stop hidden-editor animation. Repaint changed controls and visualization regions rather than the full interface. Keep all rendering, image decoding, and UI allocation out of the audio callback.

Suggested resource target: no more than about 32 MiB decoded immutable artwork at the largest supported physical scale, plus measured per-editor backdrop memory. A 3240 × 2160 RGBA image alone is about 26.7 MiB; duplicating full-resolution composites per editor can become expensive. This is a proposed engineering budget, not a measured result. Prefer smaller reusable skins/materials where they preserve the approved appearance.

## 9. Validation and definition of done

### Visual gates

- Every view uses the same lighting, material family, typography, border hierarchy, and state grammar.
- Knobs are circular, centered, sharp, and free of baked pointers; value arcs accurately follow the parameter.
- No missing controls, invented controls, clipped long names, blurry labels, alpha fringes, baked checkerboards, visible seams, or stretched bevels.
- Check all supported scales and macOS display rendering. Inspect screenshots together as a product, not as isolated attractive components.
- Browser selection, disabled controls, keyboard focus, popups, scrollbars, error messages, and warning states receive the same care as the main screen.

### Compatibility gates

Freeze hashes for processor/DSP/parameter/state/preset sources. Preserve all parameter identities/ranges/defaults, AU/VST3 IDs, automation, factory presets, state recall, latency, routing, tails, bypass, gain, and algorithm behavior. Inventory pre-existing functional issues separately; do not fold fixes into the visual branch.

Run the existing test suites and deterministic 64-program audio comparison against the chosen baseline when implementing. Compare parameter/state manifests and preset files. Exercise all 25 algorithm selections and representative algorithm-specific control sets. Verify precise entry, fine drag, reset, preset load/undo, A/B, locks, Sync, Hold, Send, Clear Tails, native DAW automation, and project recall.

### Performance gates

Use the same machine, OS, DAW, compiler, buffer, sample rate, visual mode, and scale for before/after. Measure 1/4/8/16 instances with editors open and closed. Separate normal dirty-region DAW rendering from synthetic full-paint stress. Record process CPU, GUI paint durations, frame/timer lateness, memory, and audio dropouts.

Proposed acceptance: zero new audio dropouts; closed-editor CPU indistinguishable from baseline within repeat-run noise; no sustained memory growth after repeated open/close; no repeated visible frame stalls. Investigate a repeatable median GUI-cost increase over 10% before acceptance, with absolute times reported too. Check active paint work against the existing roughly 41.7 ms update interval, accounting for shared message-thread load. These are proposed gates, not achieved results or hardware guarantees.

Previous Linux/UI Refresh passes do not validate the new artwork. Mac AU/VST3, Ableton, Retina, installer, and signing checks remain implementation/release work. Do not call the redesign sellable or Mac-validated until those checks are actually complete.

## 10. Scope, generation allowance, and delivery

Plan a first allowance of **30 outputs**, not 30 final runtime files: three material-master candidates; four pilot outputs; about seventeen production outputs including state variants; six focused corrections. Reuse accepted existing Higgsfield work only after it passes the same checks. If more states or corrections are needed, re-estimate the remaining batch; do not degrade consistency to fit an arbitrary count.

Credits are **unconfirmed**. Higgsfield catalog lookup worked, but detailed-model and 2K/4K cost-estimate calls returned internal errors. No price, turnaround guarantee, free allowance, or generation completion is claimed. Before production, obtain the exact quote for the selected model, settings, reference inputs, and batch. This planning request does not start paid generation.

Implementation sequence: freeze baseline → choose material master → pass component pilot → generate/prepare production family → theme main and all secondary views → verify audio/behavior/performance → deliver reviewable source and native build evidence when available.

Final implementation delivery should include: complete source diff on an isolated branch; Higgsfield originals and provenance; curated runtime assets; all-view before/after screenshots; protected-source comparison; tests actually run; measured before/after performance; build identity; Mac build/install instructions and validated package if produced; explicit remaining limitations.

**Success means a coherent professional Wolf Audio instrument: recognizable Tide layout, one deliberate visual language, all new artwork generated through Higgsfield, accurate live controls, attractive motion, and verified compatibility and performance.**

## Evidence links

- [Inspected source revision](https://github.com/m568zz4t2f-ui/mmm/tree/e1aea64e373a537584d6c60222fc63aa7d52859d)
- [Actual main UI screenshot](https://github.com/m568zz4t2f-ui/mmm/blob/e1aea64e373a537584d6c60222fc63aa7d52859d/docs/test-logs/ui-refresh-1.0/after/main-1080.png)
- [UI implementation](https://github.com/m568zz4t2f-ui/mmm/blob/e1aea64e373a537584d6c60222fc63aa7d52859d/Source/PluginEditor.cpp)
- [Advanced machine pages](https://github.com/m568zz4t2f-ui/mmm/blob/e1aea64e373a537584d6c60222fc63aa7d52859d/Source/ModernPanel.h)
- [Preset browser](https://github.com/m568zz4t2f-ui/mmm/blob/e1aea64e373a537584d6c60222fc63aa7d52859d/Source/PresetBrowser.h)
- [Existing Deep Current brief and asset manifest](https://github.com/m568zz4t2f-ui/mmm/tree/e1aea64e373a537584d6c60222fc63aa7d52859d/docs/deep-current)
- [Prior validation/performance report](https://github.com/m568zz4t2f-ui/mmm/blob/e1aea64e373a537584d6c60222fc63aa7d52859d/docs/UI-REFRESH-1.0.md)

Repository documents describe prior work and constraints. Higgsfield catalog recommendations describe current service capabilities; successful asset delivery and pricing still require production preflight.
