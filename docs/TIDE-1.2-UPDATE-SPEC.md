# Wolf Tide next-update specification
Date: 2026-10-27
Baseline: checked Tide 1.1.0 source. Status: implementation-ready proposal, not implemented or compiled as a new plug-in. Target version: 1.2.0, subject to compatibility validation.

## Product goal
Prioritize excellent everyday stereo delays and usable room/plate/hall sounds, then dedicated Cloud and shimmer options. Keep the grey-left delay / blue-right reverb layout, independent on/off and presets, optional combined presets, immediate dry path and dependable tails. Main page remains simple; advanced parameters are algorithm-specific.

## Evidence reviewed
Current GitHub docs/PROJECT-MEMORY.md, local Tide Source/TideDSP.h, PluginProcessor.cpp, PluginEditor.cpp, Parameters.h and update tests; Graphify Tide/Cosmos/Prism decisions; Cosmos 0.2.0 grain scheduler, renderer, coefficient caching and SpaceDSP.h. Prism details here are recalled notes rather than a fresh full source audit. No vendor proprietary DSP has been recovered.
Focused standalone Echo reproduction at 8 kHz: start at 100 ms, request 200 ms, process 80 samples, request 300 ms while crossfade is active, finish transition and inject impulse. Observed first repeat: 200 ms rather than requested 300 ms. This demonstrates a pending-target bug; it is not an audible-quality benchmark.

## A. Required fixes before new sound design
1. Delay pending target: Echo::set only initiates tap crossfades when fade>=1; Engine::set skips identical settings afterward. Move target arbitration into processing at transition boundaries so the latest requested target is applied after the current fade. Coalesce rapid updates; do not restart every sample. Keep continuous moving-head behavior for tape as intentional.
2. Air Throw preset: applyFactory writes pitchMode before its default reset loop, so Fixed Harmony is overwritten. Reset first, then apply all explicit recipe settings. Test semantic preset intent, not only consistency with the reset implementation.
3. Pitch: keep active windows/trajectories stable and transition new intervals through boundary-safe grains or bounded parallel shifters. Investigate the phase*window read-speed term and abrupt anti-alias coefficient changes. Measure before selecting a solution. Preserve linked stereo, separate filter states, and upward bandwidth limiting.
4. Pitch timing: fixed harmony maintains unshifted feedback; climbing intentionally pitches the feedback. Document wet onset and measure repeat spacing with impulses and sustained material. Compensate only predictable latency and expose unavoidable minimum wet delay rather than promising zero-latency transposition.
5. Preset identity: derive names, edited flags and scope from actual recalled state, including user presets, locks and partial recall. Do not display a factory name as an exact match when only part loaded.
6. Tail lifecycle: prepareToPlay currently reinitializes the engine. Audit repeated prepare calls at unchanged rate/layout, changes in maximum block size, reset, stop/start and changed-rate preparation. Preserve valid tails where safe; changed sample rate must reconfigure correctly.
7. Existing dry/wet and routing regressions: verify full-wet startup, send mode, direct-to-reverb, delay feed and bypass do not duplicate dry signals or unexpectedly alter level.

## B. Everyday delay architecture
Use a compact set of real engines plus reusable routing/envelope options; do not market ducking, ping-pong or slapback settings as entirely separate algorithms.

| Engine | Specification |
| --- | --- |
| Studio Digital | Clean interpolated repeats; neutral default modulation=0 and drive=0; predictable tone and lowcut; repeat timing maintained during tempo automation. |
| Tape Echo | Moving read head for tape-style changes; slow wow and faster flutter separated; controlled feedback saturation, bandwidth and repeat degradation; no claim of physical tape accuracy. |
| Analog Echo | Delay-dependent bandwidth and progressively softened repeats; restrained modulation and input/feedback coloration. Benchmark selective oversampling only for nonlinear stages; no unverified BBD claim. |
| Pattern | Upgrade Multitap with curated quarter/dotted-eighth/eighth patterns and 2/4/8-tap sets, per-tap level/pan and normalized feedback structure; stay bounded. |
| Reverse | Clear intended slice length, overlap and start/end transitions; original signal and wet timing remain explicit. |
| Pitch | Fixed Harmony and Climbing with improved transitions; common intervals front and center, +19/+24 in advanced choices. |
| Diffusion | Short allpass diffusion around repeat path; control from clear echoes to smeared ambience, with gain and decay calibrated. |
| Granular Pitch | Small bounded grain pool, stereo filtering, safe read-head lookback, smoothed overlap handling, optional onset triggering and glide shape; new sound rather than replacement for clean pitch. |
| Lo-Fi | Optional bandwidth/quantization/sample-rate coloration with controlled aliasing policy and level; texture distinct from Analog, added after core engines pass. |

Dual becomes an advanced configuration: independent A/B time/division, feedback and return level, with parallel/ping-pong choices and explicit routing. Current Dual is a channel time ratio, not two fully independent delay engines. Reuse suitable buffers, but retain independent state where behavior requires it.
Slapback (roughly 60–180 ms), dotted eighth, quarter note, stereo dual, ducked lead and swell are named recipes/options. Keep ducking and optional swell applicable to ordinary engines. Separate host tempo following, manual tempo and musical subdivision; offer tap tempo when manual timing is selected.
Retain every existing engine and old automation mapping. Resonator remains a specified follow-on model with tuned feedback/damping and bounded energy; do not crowd the first release while common delays remain unfinished.

## C. Reverb inventory and distinct structures
| Model | Design target |
| --- | --- |
| Room | Retain current engine as legacy; refine early-reflection balance, diffusion and low-level coloration for short natural spaces. |
| Plate | New refined coupled/allpass tank variant with distributed stereo taps, independently adjustable diffusion/modulation and frequency-dependent decay. Preserve old Plate sound through engine versioning. |
| Hall | Early-reflection structure plus orthogonal-matrix FDN; improve decay calibration, echo density and low/high decay control. |
| Chamber | Compact reflection geometry feeding a dense late network, short-to-medium decay and little obvious pitch movement. |
| Bloom | Deliberately delayed density buildup with controllable attack/envelope; distinguish onset from simply increasing decay. |
| Cloud Ensemble | Dedicated highly diffused late field with distributed slow modulation/ensemble and controlled early energy; no arbitrary long silence before onset. CloudSeed is a research/reference candidate, not a Strymon Cloud clone. |
| Halo Shimmer | Dedicated coupled tank and filtered pitch-feedback branch; explicit input versus regenerative contribution, pitch gain/bandwidth and musical buildup. |
| Glint | Diffusion and bloom envelope into a modulated tank, with optional subtle pitch branch; Glitz-inspired character without claiming Line 6 code. |
| Aurora Shimmer | Working replacement name for the previously proposed 'Prism' reverb to avoid confusion with the separate Wolf Prism plug-in; dedicated FDN with optional spectral pitch backend, conditional on measured latency/CPU benefit. |

Spring remains on the roadmap as a genuine dispersive/resonant model, not a bright Room preset. Complete specification, prototype and listening tests before including it. Cloud, Halo and Glint are core additions; Aurora spectral processing, Spring and Resonator have explicit quality/performance gates and must not be advertised as shipped until validated.
Common controls: decay, predelay, tone/lowcut, width, modulation, amount and ducking. Advanced controls only when implemented: early/late balance, size, diffusion, low/high decay multipliers, pitch voices and regeneration. Each model needs its own decay/energy calibration.

## D. Pitch/granular principles transferred from Cosmos
Freeze grain trajectory settings during active grains; use deterministic safe lookback for upward rates and glides; keep independent L/R filter state and linked timing where needed; crossfade Hold/reverse seams; keep a bounded voice pool.
Do not copy the full 48-voice renderer or three filtered 8-second stereo buffers into simple Tide pitch. Do not apply Cosmos multi-grain energy normalization blindly to Tide's complementary two-head Hann pair. Test coherent versus decorrelated overlap and stereo pan energy before selecting normalization.
Cosmos Room/Plate already reuse Tide tanks, so porting them back creates no new model.

## E. Preset system
One browser component, opened from left Delay, right Reverb or bottom Combined entry points. Algorithm selection is separate from named presets. Browser has Factory/User, Favorites and Recent collections; search and Guitar/Vocals/Keys/Ambient tags replace deep folders.
Visible entry: scope, name, previous/next, favorite and edited indicator. Show partial recall when locks protect settings. Mix lock protects Mix and relevant Amount controls; add Routing lock. Preserve output, Send and session tempo choices during ordinary browsing.
Implement real Undo Last Load separately from A/B snapshots. Save current scope directly into the user collection; Import/Export remains available for portable .wolftide files. Store user metadata and favorites outside factory assets. Never scan disk or parse presets in the audio callback.
Use structured explicit preset recipes for each engine rather than index-based exceptions (i==7/i==14/i==15). Save names, stable preset IDs, scope, engine IDs, sound-version metadata, tags and schema version. Validate a complete state before applying it.
Curate 32 combined musical presets (8 each Guitar, Vocal, Keys/Piano, Ambient), 4 utility presets, and 2 distinct side starters per shipped engine. Lead with Digital Quarter, Dotted Eighth, Warm Tape, Analog Lead, Stereo Dual, Vocal Slap and Duck Lead, with new ambient sounds later. These are planned counts, not completed tuning.
No automatic loudness matching during playback: curate sensible gains offline and provide a comparison workflow that preserves blend.

## F. DSP and UI optimization
1. Benchmark 1.1 baseline and the same signals/block sizes on target hardware; capture median and peak block time, allocations, memory and editor-open/closed cost.
2. Borrow Cosmos grouped coefficient invalidation: output/mix changes must not retune every tank; delay controls update delay, reverb controls update relevant tank. Handle sleeping engines' target settings correctly.
3. Retain generation-based clearing; share mutually exclusive delay/reverse storage only when state transitions remain correct. Preallocate on preparation; no callback file I/O, locks or heap allocation.
4. Derive safe quiet horizons from actual buffer path lengths; preserve 16-second Dual contents and all pitch-window propagation before sleep.
5. Only selected engine, required pitch voices and bounded outgoing tails process. Profile rapid browsing; cap retained transitions and fade oldest safely.
6. Share pitch control/table logic where useful, not independent audio histories. Keep two musical pitch voices only when selected and audible.
7. UI: retain cached artwork and static option, repaint changing regions only, no hidden analyzer or unnecessary timer; cache preset matching/status instead of repeatedly walking settings.
8. Explicit quality selection saved in state; no silent export-only algorithm changes. Spectral processing is optional, not default overhead for everyday presets.

## G. Compatibility, tests and release
Preserve legacy plug-in identity, parameter IDs/order, preset loading and old sound settings. Do not merely append host choice lists without testing normalized automation semantics: retain legacy selector domains and add a compatible extension selection or migrate with a demonstrably safe design.
Tests: pending-target reproduction; every factory intent; old state/automation; interval sweeps and chords/transients; spectral aliases; stereo channel independence and mono fold-down; freeze/clear/long tails; filter automation; repeated prepare; silence wake; full-wet/send; rapid browsing; schema validation and undo/locks; no audio-thread allocations. Preserve legacy output with a known-version fixture.
Release matrix includes 44.1/48/96/192 kHz, representative small/large/variable block sizes and actual target AU/VST3 hosts. State exactly which runs passed. Run AU validation/pluginval as applicable and at least one install/upgrade smoke test.
Deliver versioned Mac AU/VST3 installer, universal architectures if built/verified, factory assets/licenses/docs and rollback. Developer ID signing/notarization only when credentials exist; otherwise clearly label unsigned/ad-hoc state. A successful compile is not a sound-quality or installer test.

## H. Implementation order
1. Baseline fixtures and confirmed bugs; pitch transitions; same-rate prepare and routing audit.
2. Everyday delay improvements and independent A/B configuration.
3. Chamber, Cloud, Halo and Glint; refined Plate/Hall/Bloom with legacy sound preservation.
4. Granular/Diffusion and conditional Aurora/Lo-Fi; Spring/Resonator follow their gates.
5. Browser, recipes and preset curation; compatibility and CPU validation; package.
Each stage ends with measurable pass criteria, not simply a list of visible controls.

## Research references and interpretation
- Strymon TimeLine MX FAQ: https://www.strymon.net/faq/what-happened-to-the-dual-pattern-trem-duck-and-swell-delay-engines/ — supports treating duck/swell as cross-engine features and patterns as multitap behavior; does not expose DSP.
- ChowMatrix: https://github.com/Chowdhury-DSP/ChowMatrix — independent delay-node controls and interpolation research; BSD-3-Clause main project, audit dependencies separately.
- Dattorro: https://ccrma.stanford.edu/~dattorro/EffectDesignPart1.pdf — published plate-like tank principles, not proprietary source recovery.
- Signalsmith reverb design: https://signalsmith-audio.co.uk/writing/2021/lets-write-a-reverb/ — orthogonal feedback mixing, diffusion and decay design.
- CloudSeedCore: https://github.com/GhostNoteAudio/CloudSeedCore — MIT core, attribution required; core is open but commercial plug-in UI is not.
- Cloud Seed guide: https://ghostnoteaudio.uk/pages/cloud-seed-user-guide — separate early/late sections, diffusion density, modulation and size/damping interaction; use bounded tuning rather than cloning maximum tap counts.
- Signalsmith Stretch: https://github.com/Signalsmith-Audio/signalsmith-stretch — evaluate polyphonic pitch, explicit latency and CPU peaks.
- Cosmos code and Graphify lessons are internal references, not evidence of exact Microcosm behavior.
Reverse engineering continues as a separate research track; no firmware recovery or commercial cloning claim is required for this release.
