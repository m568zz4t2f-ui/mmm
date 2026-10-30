# Deep Current implementation and acceptance

Starting commit: `e1aea64e373a537584d6c60222fc63aa7d52859d` on `design/tide-deep-current`.
Implementation branch: `development/tide-deep-current`.
Comparison with `464e0c3f7047c2a50198a824b1cbaf61caed4bee`: only design documentation, reference previews and asset-import workflow differ. Processor and working editor source are identical.

## Resolved requirements

The new request asks for a complete interface overhaul. Both supplied master prompts require existing coordinates, controls and bindings to stay intact. Deep Current therefore changes materials, typography, controls and visual rendering across existing views, preserving the existing layout and interaction code. No feature, parameter, preset, processor or algorithm fixes belong to this branch.

The uploaded master prompt is more detailed than the shorter repository prompt; they agree on scope. Use `development/tide-deep-current` as the explicitly requested equivalent of the uploaded prompt's suggested branch name. System sans is permitted by the uploaded prompt, avoiding an extra font dependency and redistributable font files. Native Mac acceptance cannot be established on this Linux build host.

## Requirement map

| Requirement | Authority | Implementation / evidence |
| --- | --- | --- |
| Frozen source and working baseline | Master prompt; QA protected baseline | `evidence/baseline/`, protected SHA-256 manifest and source inventory |
| Same layout, input behavior and bindings | Both master prompts; design system | Main `resized`, attachments and callbacks retained; geometry inventory comparison |
| Graphite left, teal right, satin chassis | Design system: panels | Clean source-material crops; shared cached faceplate |
| One functional typography family | Design system: typography | JUCE platform sans, consistent weights; original italic Tide branding |
| Shared smoked titanium knobs | Design system: knobs | Source 05 cleaned face; live vector arc and pointer, bounded physical-size cache |
| Live delay taps and reverb waves | Design system: screens | Existing primary-engine data and wave paths retained; refined glass/bezel |
| Advanced pages, browser, menus, dialogs | Master prompt: all views | Shared `TideLook`, five independent pages, three legacy expanded pages, browser scopes |
| Embed and package offline resources | User plan sections 9, 13 | Explicit BinaryData list, asset-aware build ID, archive includes assets and pinned JUCE |
| All protected files and 206 presets identical | QA protected baseline | Guard script, all-program matched audio fixture, CTest |
| Measured rendering and memory | PERFORMANCE.md | Separate normal-window and forced-paint trials; native Mac gate stays open |
| Mac build and conflict/rollback tools | QA native blockers; user plan | Existing universal build/installer tools packaged; native execution pending |

## Artwork decisions

All original images were inspected at full resolution (metadata and individual material/knob/screen inspection) and as a labeled contact sheet. Their dimensions match the supplied manifest. RGBA source opacity peaks at 249–254 rather than 255, and several silhouettes contain speckles; none are embedded without preparation.

| Source | Role | Ships in runtime |
| --- | --- | --- |
| 00 | Composition/material moodboard; invented layout excluded | No |
| 01 | Inner unlettered blue-black satin shell | Cropped chassis |
| 02 | Blank material inside graphite panel; generated recess geometry excluded | Cropped graphite |
| 03 | Blank teal material; generated knobs, fixed arcs and wave ornament excluded | Cropped teal |
| 04 | Footer surface proportions/material reference | Vector footer |
| 05 | Main machined face; fixed needle repaired, silhouette alpha cleaned | Titanium face |
| 06 | Small-control material reference; one shared family retained | Same 05 face at secondary size |
| 07 | Delay bezel reference | Vector bezel |
| 08 | Reverb bezel and empty glass reference | Cropped glass, vector bezel |
| 09 | Reflection reference | Restrained cached vector reflection; raw overlay excluded |
| 10 | Button state material reference; sheet contains knobs | Vector buttons, no sheet sprites |
| 11 | Selector shell reference | Vector selector shells |
| 12 | Linear control reference | Vector rail and machined thumb |
| 13 | State/icon reference with inaccurate glyph geometry | Existing text, clean vector tick/chevron |
| 14 | Current motif reference | Restrained code-drawn line motif |

Production sizes, exact source SHA-256, crops, padding, encoded and decoded bytes are in `assets/tide/deep-current/production/manifest.json`. Only five 2× PNGs are embedded. 1× exports travel with corresponding source. Decoded immutable resources are shared; resizing is cached separately. No raw contact sheet enters the runtime. Full original pack remains available from the manifest URLs and uploaded pack; repository previews retain all 15 references.

## Existing behavior kept separate

Primary displays illustrate Delay A and Reverb A rather than all four slots. Existing legacy capability flags, preset/state handling and transport-tail behavior remain outside scope. Existing source has compact statements that trigger compiler indentation warnings; these are not UI defects and are retained. Native file pickers stay native.

## Isolated visual corrections

The existing backdrop cache used only image dimensions. Switching directly between expanded legacy pages at the same window size could retain the previous page heading. The candidate includes the page identity in its cache key; this changes only the printed heading/backdrop, with the original callbacks and coordinates unchanged. Plugin-owned alerts explicitly receive the shared theme, and that module-lifetime theme remains valid after an editor closes. No audio or preset defect is addressed here.
