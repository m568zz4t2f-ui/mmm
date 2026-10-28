# Wolf Tide 1.3 — Unified Overhaul

One comprehensive update following 1.2.2 · DSP, algorithms, routing, presets, interface, and optimization

Instead of spreading improvements across versions 1.3, 1.4, 1.5, and 2.0, we should consolidate everything into one major Wolf Tide 1.3 update.

The central objective is to transform Tide into a more sophisticated, professional delay and reverb plugin, with genuinely distinctive algorithms for every effect type, improved sound quality, flexible routing, consistent output levels, and a cleaner interface.

We will preserve the fixes already underway in 1.2.2 and use that version as our starting point.

## 1. Unified update priorities

| Priority | Development area           | What changes                                                                |
| -------- | -------------------------- | --------------------------------------------------------------------------- |
| P0       | Algorithm differentiation  | Redesign and refine all 20 existing effect types                            |
| P0       | Sound quality              | Improve clarity, modulation, stereo imaging, decay and feedback             |
| P1       | Gain staging               | Calibrate loudness and avoid clipping across models                         |
| P1       | Dual engines               | Two independent delays and/or reverbs with flexible routing                 |
| P1       | Advanced controls          | Meaningful, machine-specific parameters                                     |
| P1       | Presets                    | Retune factory sounds, improve recall and preserve compatibility            |
| P2       | New effects                | Oil Can, Spectral, Drum Echo, Nonlinear and other distinctive effects       |
| P2       | Interface                  | Cleaner boutique design, better navigation and dynamic controls             |
| P0       | Stability and optimization | CPU efficiency, seamless switching, host compatibility and regression tests |

## 2. Algorithm overhaul — the biggest part

Every effect must have a unique audible purpose. Shared DSP utilities are fine, but simply changing filter values or diffusion settings is not enough.

### All 9 existing delay algorithms

| Effect         | Required changes                                                                               |
| -------------- | ---------------------------------------------------------------------------------------------- |
| Digital        | High-fidelity, transparent repeats, precise timing and flexible stereo behavior                |
| Tape           | Proper tape-style head behavior, saturation, irregular wow/flutter, filtering and age controls |
| Bucket         | BBD-inspired clock behavior, companding, progressive bandwidth loss and nonlinear feedback     |
| Multitap       | Independently adjustable taps, rhythm, level, pan and feedback                                 |
| Reverse        | More natural reverse phrases, smoother crossfades and adjustable attack                        |
| Pitch          | Better transposition quality, harmonic feedback and reduced pitch artifacts                    |
| Diffusion      | Dedicated diffusion network with adjustable density, texture and stereo spread                 |
| Granular Pitch | Improved grain scheduling, overlap, position, pitch voices and freeze behavior                 |
| Lo-Fi          | Independent sample-rate reduction, bit depth, noise, filtering and degradation controls        |

### All 11 existing reverb algorithms

| Effect         | Required changes                                                                |
| -------------- | ------------------------------------------------------------------------------- |
| Room           | Realistic reflection geometry and natural spatial decay                         |
| Plate          | Dense dispersion and smooth metallic plate character                            |
| Hall           | Detailed early reflections, spacious late decay and frequency-dependent damping |
| Bloom          | Evolving buildup with smooth sustained tails                                    |
| Chamber        | Distinct chamber geometry and tight, dense reflections                          |
| Cloud Ensemble | Dedicated highly diffused, evolving ambient reverb                              |
| Halo Shimmer   | Cleaner pitch shifting, harmonic layering and controlled feedback               |
| Glint          | Bright, textured, evolving reflections with distinctive attack behavior         |
| Studio Plate   | Higher-density, specialized plate architecture                                  |
| Studio Hall    | Expanded spatial architecture and sophisticated late-tail modulation            |
| Studio Bloom   | Independent swell and buildup processing with evolving diffusion                |

Most important architectural fix: Replace the current heavy dependence on the shared `BoutiqueTank` configuration with more specialized processing paths where they create meaningful sonic differences.

## 3. Add five distinctive effect families

New algorithms should be implemented only when they introduce processing behavior that the existing types cannot adequately produce.

| New algorithm      | Main characteristic                                       |
| ------------------ | --------------------------------------------------------- |
| Oil Can Delay      | Fluid, warbling vintage echo                              |
| Spectral Delay     | Frequency-dependent echoes and evolving harmonic textures |
| Magnetic Drum Echo | Multi-head rhythmic patterns and magnetic coloration      |
| Nonlinear Reverb   | Gated, shaped and reverse-style reverb envelopes          |
| Resonator Reverb   | Tuned harmonic reflections and sustained resonances       |

These should be developed using original DSP designs informed by documented signal-processing techniques, not copied proprietary algorithms.

## 4. Dual-engine architecture and routing

This should be part of 1.3, rather than a separate future release.

Proposed engine routing

SeriesParallelSplit

Both engines run independently and blend together. Conceptual design only.

Required capabilities include independent delay A/B and reverb A/B, series/parallel/split routing, separate feedback and modulation, stereo control, tempo synchronization, independent bypass, cross-feedback where stable, and smooth spillover.

To preserve efficiency, single-engine presets should run only the required engines.

## 5. Unified sound-quality improvements

These improvements apply across every algorithm:

- Output consistency: Match perceived levels without flattening dynamics.
- Headroom: Prevent clipping and runaway feedback with appropriate gain staging and bounded nonlinearities.
- Stereo depth: More interesting width, decorrelation and motion while maintaining mono compatibility.
- Modulation: Smooth, low-artifact movement without repetitive or unnatural wobble.
- Pitch quality: Reduce windowing artifacts and improve transposition clarity.
- Tails: Eliminate abrupt cuts, unnatural ringing, retrigger suppression and discontinuities.
- Transitions: Smooth model changes, delay-time changes, bypass and tempo changes.
- Responsiveness: Ensure every exposed parameter produces an audible or clearly documented effect in its intended mode.

The goal is for every effect to sound polished on both isolated notes and complete musical performances.

## 6. Presets and interface — synchronized with 1.2.2

We should carry forward the entire preset and version-display repair from 1.2.2.

The new architecture must maintain:

- Correct plugin version and build identification.
- Immediate synchronization of preset names and active effect types.
- Proper Factory/User/Modified status.
- Machine-specific controls that appear or become available only when relevant.
- Correct behavior for scoped delay, reverb and combined presets.
- Lock states, undo, A/B comparisons and host automation.
- Reliable old-project and old-preset migration.
- Searchable and categorized presets with favorites.
- A cleaner main panel with optional deeper editing.

Factory presets must be retuned for the revised algorithms, not merely carried forward unchanged.

## 7. Efficiency and compatibility requirements

This is essential because the combined scope of the update is large.

| Area             | Engineering requirement                                                                         |
| ---------------- | ----------------------------------------------------------------------------------------------- |
| Realtime safety  | Avoid audio-thread allocations, locks, file I/O and other blocking operations                   |
| CPU              | Profile each machine individually and in dual-engine combinations                               |
| Memory           | Allocate only required processing resources where safely possible                               |
| Feedback         | Bounded and stable across extreme configurations                                                |
| Automation       | Smooth parameter changes without pops or unexpected jumps                                       |
| Sample rates     | Test 44.1, 48, 96 and 192 kHz, plus low-rate safety cases                                       |
| Host integration | Validate AU, VST3 and standalone                                                                |
| Compatibility    | Preserve existing parameter identity and support old session states                             |
| Installation     | Avoid user/system duplicate-plugin conflicts and retain rollback copies                         |
| DSP validation   | Impulses, frequency responses, decay measurements, stereo correlation and real musical material |

For important existing sounds, I would preserve a legacy processing path or another reliable migration mechanism so projects do not silently change when the new algorithms are introduced.

## 8. Efficient implementation sequence — one release

Even though everything ships together, the engineering should be organized into dependencies to avoid repeatedly rewriting the same code.

1. Foundation and baseline

   Integrate the finalized 1.2.2 fixes, freeze compatibility requirements, establish measurements for every existing effect and identify which code can be reused safely.
2. Core DSP infrastructure

   Build reusable high-quality delay lines, interpolation, modulation, diffusion, filters, pitch utilities, gain management and engine-lifecycle controls.
3. Algorithm redevelopment

   Improve all nine delays and eleven reverbs, prioritizing the most generic or least differentiated architectures.
4. New machines and dual engines

   Implement the distinctive new effect families, independent engines and routing matrix on top of the improved DSP infrastructure.
5. Interface and preset integration

   Expose appropriate parameters, synchronize displays, implement routing controls and redesign factory sounds.
6. Full-system testing and optimization

   Measure quality, stability, CPU, memory, output consistency and DAW compatibility; fix regressions before packaging.

## 9. Release acceptance criteria

Wolf Tide 1.3 should not be considered finished merely because every effect compiles.

It should only ship after demonstrating that:

- Every existing machine has a distinct, documented audible purpose.
- Added machines do something meaningfully new.
- All controls function correctly for their selected machine.
- Output levels and stereo behavior are consistent and predictable.
- Dual-engine routing works without clicks, unintended feedback or broken tails.
- All preset and host-state workflows remain reliable.
- New algorithms are tested musically against appropriate references.
- Existing projects retain a supported compatibility path.
- Automated DSP tests and host validation pass.
- The universal macOS installer builds, validates and preserves rollback options.

## Final consolidated plan

## Wolf Tide 1.3 — Complete DSP & Workflow Overhaul

# 20

Existing machines to refine

# 5

Proposed new effect families

# 4

Possible engine slots: delay A/B, reverb A/B

# 1

Unified release

Primary objective: Make the algorithms themselves substantially better and more distinctive, while upgrading routing, presets, workflow and performance together.

This is one coordinated 1.3 update, not multiple feature releases. The development phases are internal milestones; the finished improvements ship together after validation.

No live plugin code has been changed yet.